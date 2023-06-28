<?hh

function dump_hash() :mixed{
  $hash = HH\ImplicitContext\_Private\get_implicit_context_memo_key();
  echo "Hash: " . quoted_printable_encode($hash) . "\n";
}

<<__Memoize(#SoftMakeICInaccessible)>>
function soft_ic_inaccessible($x) :mixed{
  dump_hash();
  HH\ImplicitContext\soft_run_with(
    () ==> soft_ic_inaccessible2($x),
    "SOFT_KEY2"
  );
}

<<__Memoize(#SoftMakeICInaccessible)>>
function soft_ic_inaccessible2($x) :mixed{
  dump_hash();
  echo "ok\n";
}

function p($text) :mixed{
  echo ">>>>> " . $text . "\n";
}

function handler($errno, $errstr, $errfile, $errline, $errcontext, $backtrace, $ic_blame) :mixed{
  echo $errstr . " on " . $errfile . ":" . $errline . "\n";
  echo "Blame is:\n";
  var_dump($ic_blame);
}

<<__EntryPoint>>
function main() :mixed{
  include 'implicit.inc';
  set_error_handler(handler<>);

  p("from soft inaccessible");
  soft_ic_inaccessible(0);
  p("from soft run with");
  HH\ImplicitContext\soft_run_with(
    () ==> soft_ic_inaccessible(1),
    "SOFT_KEY1"
  );
}
