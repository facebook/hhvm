<?hh

function foo(inout $x) {}

abstract final class MainStatics {
  public static $x;
}

function main() {
  foo(inout MainStatics::$x[1][2][3]);
}


<<__EntryPoint>>
function main_bad_call_11() {
main();
}
