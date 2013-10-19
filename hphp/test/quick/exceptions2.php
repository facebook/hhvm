<?php
function newobj() {
  return new C;
}
global $constructCount;
global $destructCount;
class C {
  public function __destruct() {
    global $y;
    $y = false;
    global $destructCount;
    ++$destructCount;
  }
  public function g($x, $y, $z) {
    return $y;
  }
  public function foo($z) {
    global $constructCount;
    ++$constructCount;
    $w1 = 1;
    $w2 = 2;
    newobj()->g("hi", $w1, 7, 8, $w2 = $this->foo(newobj()));
  }
}
function bar() {
  $obj = new C;
  newobj()->foo(123);
}
function onShutdown() {
  // What we really want to know is that there were a reasonable
  // number of calls to the destructor. We count the objects we leave
  // on the stack, and expect the destruct count to be quite close to
  // that.
  global $constructCount;
  global $destructCount;
  if (abs($destructCount - $constructCount) < 2) {
    echo "Saw a reasonable number of calls to C::__destruct()\n";
  } else {
    echo "Unexpected number of calls to C::__destruct(): $destructCount\n";
  }
}
register_shutdown_function('onShutdown');
bar();
