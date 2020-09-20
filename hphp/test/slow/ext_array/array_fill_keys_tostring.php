<?hh

class StringableObj { function __toString() { return 'Hello'; } }

<<__EntryPoint>>
function main() {
  array_fill_keys(varray[new StringableObj()], 'value');
}
