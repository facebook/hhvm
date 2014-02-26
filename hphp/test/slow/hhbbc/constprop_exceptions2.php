<?php

class Heh {
  public function __toString() { throw new Exception('heh'); }
}

// We need to hide some of these things in functions or hphpc has the
// same bug for exceptional control flow that this test is intended to
// catch in hhbbc.  (We need to make sure we propagate the state
// before the SetN to the catch block, since it can throw from the
// object __toString.).
function get_two() { return 2; }
function do_echo($x) { echo $x . "\n"; }

function get_thing() {
  return new Heh();
}

function foo() {
  $k = get_thing();
  $y = get_two();
  $x = "initial x value";
  do_echo($x);
  try {
    $x = $y ? "ok1" : "ok2";
    ${$k} = 3;
    goto heh; // needed or hphpc decides the assignment to $x can be removed
  } catch (Exception $e) {
    var_dump($x);
    return;
  }

heh:
  var_dump($x);
}

foo();
foo();
