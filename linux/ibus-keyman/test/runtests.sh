#!/bin/bash

tests=(
  '000 - null keyboard'
  '001 - basic input UnicodeI'
  '002 - basic input Unicode'
  '003 - nul'
  '004 - basic input (shift 2)'
  '005 - nul with initial context'
  '006 - vkey input (shift ctrl)'
  '007 - vkey input (ctrl alt)'
  '008 - vkey input (ctrl alt 2)'
  '012 - ralt'
  '013 - deadkeys'
  '014 - groups and virtual keys'
  '015 - ralt 2'
  '017 - space mnemonic kbd'
  '018 - nul testing'
  '019 - multiple deadkeys'
  '020 - deadkeys and backspace'

  # '021 - options'
  # '022 - options with preset'
  # '023 - options with save'
  # '024 - options with save and preset'
  # '025 - options with reset'

  # '026 - system stores'
  # '027 - system stores 2'

  '028 - smp'
  '029 - beep'

  '030 - multiple groups'
  # '031 - caps lock'
  # '032 - caps control'
  # '033 - caps always off'
  # '034 - options double set reset'
  # '035 - options double set staged'
  # '036 - options - double reset staged'
  # '037 - options - double reset'
  '038 - punctkeys'
  '039 - generic ctrlalt'
  # '040 - long context'
  # '041 - long context and deadkeys'
  # '042 - long context and split deadkeys'
  # '043 - output and keystroke'

  # '044 - if and context'
  # '045 - deadkey and context'
  # '046 - deadkey and contextex'
  # '047 - caps always off initially on'
  )


#echo ${tests[*]}
#echo $tests[*]

#for test in ${tests}; do echo "$test"; done

if [ ! -d ~/.local/share/keyman/test_kmx ]; then
  ln -sf $(pwd)/../../../common/core/desktop/tests/unit/kmx ~/.local/share/keyman/test_kmx
fi

model=`setxkbmap -query|grep model|cut -d ':' -f 2|tr -d ' '`
layout=`setxkbmap -query|grep layout|cut -d ':' -f 2|tr -d ' '`
rules=`setxkbmap -query|grep rules|cut -d ':' -f 2|tr -d ' '`
variant=`setxkbmap -query|grep variant|cut -d ':' -f 2|tr -d ' '`

echo "System xkb keyboard"
echo "model:$model"
echo "layout:$layout"
echo "rules:$rules"
echo "variant:$variant"

echo "Setting keyboard to us"
setxkbmap us
setxkbmap -query

export PYTHONPATH=$(pwd)/../../keyman-config
export KEYMAN_NOSENTRY=1

count=0
passed=0
failed=0
while [ "x${tests[count]}" != "x" ]
do
  echo ${tests[count]}
  setxkbmap us
  python3 test_ibus_keyman.py "${tests[count]}"
  diffret=`diff -uws "${tests[count]}.in" "${tests[count]}.out"`
  # echo $?
  if [ "x$?" == "x0" ]; then
    echo "passed"
    passed=$(( $passed + 1 ))
  else
    echo "failed"
    failed=$(( $failed + 1 ))
  fi
  count=$(( $count + 1 ))
done

echo "Setting xkb keyboard back to system one"
if [ "x$variant" == "x" ]; then
  setxkbmap -model $model -layout $layout -rules $rules
else
  setxkbmap -model $model -layout $layout -rules $rules -variant $variant
fi
setxkbmap -query

echo "-----------------------------------------------------"
echo "Tests run : ${count}"
echo "Passed    : ${passed}"
echo "Failed    : ${failed}"
