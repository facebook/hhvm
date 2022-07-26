<?hh

function dump_ctx_data() {
  try {
    echo ClassContext::getContext()->name() . "\n";
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
}

function dump_hash() {
  $hash = HH\ImplicitContext\_Private\get_implicit_context_memo_key();
  echo "Hash: " . quoted_printable_encode($hash) . "\n";
}

class Counter {
  static int $noArg = 0;
  static int $withArg = 0;
}

<<__Memoize(#KeyedByIC)>>
function memoNoArg() {
  dump_ctx_data();
  dump_hash();
  Counter::$noArg++;
  return 1;
}

<<__Memoize(#KeyedByIC)>>
function memoWithArg($x) {
  dump_ctx_data();
  dump_hash();
  Counter::$withArg++;
  return 1;
}

function f() {
  $v = vec[
    tuple('memoNoArg', () ==> memoNoArg()),
    tuple('memoWithArg', () ==> memoWithArg(1)),
  ];
  foreach ($v as list($name, $f)) {
    echo "  Before: "; dump_ctx_data();
    try {
      $f();
      echo $name . " passed\n";
    } catch (Exception $e) {
      echo $name . " failed with: " . $e->getMessage() . "\n";
    }
    echo "  After: "; dump_ctx_data();
    echo "------------------------\n";
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

function p($text) {
  echo ">>>>> " . $text . "\n";
}

<<__EntryPoint>>
function main() {
  include 'implicit.inc';
  p("from null");
  f();
  p("from value");
  ClassContext::start(new C, f<>);
  p("from inaccessible");
  ic_inaccessible();
  p("from soft inaccessible");
  soft_ic_inaccessible();
  p("from soft run with");
  HH\ImplicitContext\soft_run_with(f<>, "SOFT_KEY");
  p("final report");
  echo "No arg ran: " . Counter::$noArg . " times\n";
  echo "With arg ran: " . Counter::$withArg . " times\n";
}
