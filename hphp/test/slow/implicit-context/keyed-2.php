<?hh

class Counter {
  static int $noArg = 0;
  static int $withArg = 0;
}

<<__Memoize(#KeyedByIC)>>
function memoNoArg() {
  Counter::$noArg++;
  return 1;
}

<<__Memoize(#KeyedByIC)>>
function memoWithArg($x) {
  Counter::$withArg++;
  return 1;
}

function f() {
  $v = vec[
    () ==> memoNoArg(),
    () ==> memoWithArg(1),
  ];
  foreach ($v as $f) {
    try { $f(); } catch (Exception $_) {}
  }
}

<<__Memoize(#MakeICInaccessible)>>
function ic_inaccessible() {
  f();
}

<<__Memoize(#SoftMakeICInaccessible)>>
function soft_ic_inaccessible() {
  f();
}

<<__EntryPoint>>
function main() {
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
