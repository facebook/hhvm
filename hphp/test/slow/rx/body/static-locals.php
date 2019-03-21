<?hh

<<__Rx>>
function test() {
  static $x = 1;         // StaticLocInit
  $x++; // prevent dce from eating $x
  static $y = Vector{1}; // StaticLocCheck, StaticLocDef
}

<<__EntryPoint>>
function main() {
  test();
  echo "Done\n";
}
