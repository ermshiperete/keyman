# ldml-keyboards

Data is versioned, so each directory has its version such as `45`.

## Data Format

Each directory contains:

- `ldmlKeyboard3.dtd` - the DTD file
- `ldmlKeyboard3.xsd` - the XSD file, automatically converted from the DTD using
  Visual Studio, hand tweaked as necessary
- `ldml-keyboard3.schema.json` - the JSON schema file, automatically converted
  from the XSD using xsd2json (https://github.com/Mermade/jgeXml), hand tweaked
  as necessary:
    - change toplevel "id" to "$id"
    - add "type": "object" to /keyboard
    - change /keyboard/keys/key type to "array"

- `imports/` - the importable data files (TODO-LDML)

## License

- [unicode-license.txt](unicode-license.txt) - Unicode license
