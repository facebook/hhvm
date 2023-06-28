<?hh

function dump_ctx_data() :mixed{
  try {
    echo ClassContext::getContext()->name() . "\n";
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
}

function dump_hash() :mixed{
  $hash = HH\ImplicitContext\_Private\get_implicit_context_memo_key();
  echo "Hash: " . quoted_printable_encode($hash) . "\n";
}

class Counter {
  public static int $noArg = 0;
  public static int $withArg = 0;
}

<<__Memoize(#KeyedByIC)>>
function memoNoArg() :mixed{
  dump_ctx_data();
  dump_hash();
  Counter::$noArg++;
  return 1;
}

<<__Memoize(#KeyedByIC)>>
function memoWithArg($x) :mixed{
  dump_ctx_data();
  dump_hash();
  Counter::$withArg++;
  return 1;
}

function f() :mixed{
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
function ic_inaccessible() :mixed{
  f();
}

<<__Memoize(#SoftMakeICInaccessible)>>
function soft_ic_inaccessible() :mixed{
  f();
}

function p($text) :mixed{
  echo ">>>>> " . $text . "\n";
}

<<__EntryPoint>>
function main() :mixed{
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
