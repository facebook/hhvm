<?hh

enum E : arraykey {
  NAME1 = '1';
  NAME2 = '2';
  NAME3 = 1;
}

<<__EntryPoint>>
function main() {
  var_dump(E::getNames());
}
