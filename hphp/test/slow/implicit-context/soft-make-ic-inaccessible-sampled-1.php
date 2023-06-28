<?hh

<<__Memoize(#SoftMakeICInaccessible, 100000000)>>
function soft_ic_inaccessible($x) :mixed{
  trigger_error("inside with $x");
}

<<__Memoize(#SoftMakeICInaccessible, 1)>>
function soft_ic_inaccessible_1($x) :mixed{
  trigger_error("inside with $x");
}

function handler($errno, $errstr, $errfile, $errline,
                 $errcontext, $backtrace, $ic_blame) :mixed{
  echo "$errstr\n";
  var_dump($ic_blame);
}

<<__EntryPoint>>
function main() :mixed{
  set_error_handler(handler<>);
  soft_ic_inaccessible(1);
  soft_ic_inaccessible(2);
  soft_ic_inaccessible(3);
  soft_ic_inaccessible(4);

  soft_ic_inaccessible_1(1);
  soft_ic_inaccessible_1(2);
}
