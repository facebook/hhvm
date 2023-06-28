<?hh

function foo(inout $x) :mixed{}

abstract final class MainStatics {
  public static $x;
}

function main() :mixed{
  foo(inout MainStatics::$x[1][2][3]);
}


<<__EntryPoint>>
function main_bad_call_11() :mixed{
main();
}
