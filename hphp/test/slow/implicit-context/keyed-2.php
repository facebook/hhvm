<?hh

class Counter {
  public static int $noArg = 0;
  public static int $withArg = 0;
}

<<__Memoize(#KeyedByIC)>>
function memoNoArg() :mixed{
  Counter::$noArg++;
  return 1;
}

<<__Memoize(#KeyedByIC)>>
function memoWithArg($x) :mixed{
  Counter::$withArg++;
  return 1;
}

function f() :mixed{
  $v = vec[
    () ==> memoNoArg(),
    () ==> memoWithArg(1),
  ];
  foreach ($v as $f) {
    try { $f(); } catch (Exception $_) {}
  }
}

<<__Memoize(#MakeICInaccessible)>>
function ic_inaccessible() :mixed{
  f();
}

<<__Memoize(#SoftMakeICInaccessible)>>
function soft_ic_inaccessible() :mixed{
  f();
}

<<__EntryPoint>>
function main() :mixed{
  include 'implicit.inc';
  for ($i = 0; $i < 4; $i++) {
    f();
    ClassContext::start(new C, f<>);
    ic_inaccessible();
    soft_ic_inaccessible();
    HH\ImplicitContext\soft_run_with(f<>, "SOFT_KEY");
  }
  echo "No arg ran: " . Counter::$noArg . " times\n";
  echo "With arg ran: " . Counter::$withArg . " times\n";
}
