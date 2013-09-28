<?php

class Obj {
  public function __destruct() {
    // Raise a fatal.
    class Obj {
}
  }
}

function foo() {
  $y = new Obj;
  $x = new Obj;
  $y = new Obj;

  // Currently our behavior during return when a local dtor throws a
  // fatal is to swallow it, then keep rethrowing it from the enter
  // hook for each destructor.  Then we return as normal (and this
  // test is trying to make sure nothing chokes due to the destroyed
  // locals), then further out the next enter hook will throw the
  // fatal.
}

try {
  foo();
}
 catch (Exception $x) {
 echo "notreached\n";
 }
foo();
 // enter hook throws the fatal

