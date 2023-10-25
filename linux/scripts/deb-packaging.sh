#!/usr/bin/env bash
# Actions for creating a Debian source package. Used by deb-packaging.yml GHA.

set -eu

## START STANDARD BUILD SCRIPT INCLUDE
# adjust relative paths as necessary
THIS_SCRIPT="$(readlink -f "${BASH_SOURCE[0]}")"
. "${THIS_SCRIPT%/*}/../../resources/build/build-utils.sh"
## END STANDARD BUILD SCRIPT INCLUDE

builder_describe \
  "Helper for building Debian packages." \
  "dependencies              Install dependencies as found in debian/control" \
  "source+                   Build source package" \
  "verify                    Verify API" \
  "--gha                     Build from GitHub Action" \
  "--src-pkg=SRC_PKG         Path and name of source package (for verify action)" \
  "--bin-pkg=BIN_PKG         Path and name of binary Debian package (for verify action)" \
  "--pkg-version=PKG_VERSION The version of the Debian package (for verify action)" \
  "--git-ref=GIT_REF         The ref of the HEAD commit, e.g. HEAD of the PR branch (for verify action)" \
  "--git-base=GIT_BASE       The ref of the base commit, e.g. HEAD of the master branch (for verify action)"

builder_parse "$@"

cd "$REPO_ROOT/linux"

if builder_has_option --gha; then
  START_STEP="::group::${COLOR_GREEN}"
  END_STEP="::endgroup::"
else
  START_STEP="${COLOR_GREEN}"
  END_STEP=""
fi

dependencies_action() {
  sudo mk-build-deps --install --tool='apt-get -o Debug::pkgProblemResolver=yes --no-install-recommends --yes' debian/control
}

source_action() {
  echo "${START_STEP}Make source package for keyman${COLOR_RESET}"

  echo "${START_STEP}reconfigure${COLOR_RESET}"
  ./scripts/reconf.sh
  echo "${END_STEP}"

  echo "${START_STEP}Make origdist${COLOR_RESET}"
  ./scripts/dist.sh origdist
  echo "${END_STEP}"

  echo "${START_STEP}Make deb source${COLOR_RESET}"
  ./scripts/deb.sh sourcepackage
  echo "${END_STEP}"

  mv builddebs/* "${OUTPUT_PATH:-..}"
}

check_api_not_changed() {
  # Checks that the API did not change compared to what's documented in the .symbols file
  tmpDir=$(mktemp -d)
  dpkg -x "${BIN_PKG}" "$tmpDir"
  cd debian
  dpkg-gensymbols -v"${PKG_VERSION}" -p"${PKG_NAME}" -e"${tmpDir}"/usr/lib/x86_64-linux-gnu/"${LIB_NAME}".so* -O"${PKG_NAME}.symbols" -c4
  echo ":heavy_check_mark: ${LIB_NAME} API didn't change" >&2
  builder_echo green "OK ${LIB_NAME} API didn't change"
}

check_updated_version_number() {
  # Checks that the version number got updated in the .symbols file if it got changed
  if [[ $(git rev-list -1 "${GIT_REF}" -- "linux/debian/${PKG_NAME}.symbols") != $(git rev-list -1 "${GIT_BASE}" -- "linux/debian/${PKG_NAME}.symbols") ]]; then
    # .symbols file changed, now check if the version got updated as well
    if ! git log -p -1 -- "linux/debian/${PKG_NAME}.symbols" | grep -q "${PKG_VERSION}"; then
      echo ":x: ${PKG_NAME}.symbols file got changed without changing the version number of the symbol" >&2
      builder_echo error "Error: ${PKG_NAME}.symbols file got changed without changing the version number of the symbol"
      exit 1
    fi
    echo ":heavy_check_mark: ${PKG_NAME}.symbols file got updated with version number" >&2
    builder_echo green "OK ${PKG_NAME}.symbols file got updated with version number"
  fi
}

verify_action() {
  tar xf "${SRC_PKG}"
  PKG_NAME=libkeymancore
  LIB_NAME=libkeymancore
  if [ ! -f debian/${PKG_NAME}.symbols ]; then
    echo ":warning: Missing ${PKG_NAME}.symbols file" >&2
    builder_echo warning ":warning: Missing ${PKG_NAME}.symbols file"
    exit 0
  fi
  check_api_not_changed
  check_updated_version_number
}

builder_run_action dependencies  dependencies_action
builder_run_action source        source_action
builder_run_action verify        verify_action
