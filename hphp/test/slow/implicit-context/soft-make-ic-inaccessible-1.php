<?hh

<<__Memoize(#SoftMakeICInaccessible)>>
async function soft_ic_inaccessible() :Awaitable<mixed>{
  return 1;
}

class A {
  <<__Memoize(#SoftMakeICInaccessible)>>
  static async function soft_ic_inaccessible() :Awaitable<mixed>{
    return 1;
  }
}

function handler($errno, $errstr, $errfile, $errline,
                 $errcontext, $backtrace, $ic_blame) :mixed{
  echo "$errstr\n";
  echo "Blame is:\n";
  var_dump($ic_blame);
}

<<__EntryPoint>>
async function main() :Awaitable<mixed>{
  set_error_handler(handler<>);
  trigger_error("before");
  soft_ic_inaccessible();
  trigger_error("middle");
  A::soft_ic_inaccessible();
  trigger_error("after");
}
