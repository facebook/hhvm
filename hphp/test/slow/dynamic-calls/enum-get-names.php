<?hh

enum E : int {
  FOO = 42;
  BAR = 42;
}

<<__EntryPoint>>
function main_enum_get_names(): void {
  E::getNames();
  echo "FAIL\n";
}
