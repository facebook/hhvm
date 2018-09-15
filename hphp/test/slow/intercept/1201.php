<?php

// Make sure that we can tell which class was called for intercepted static
// methods

class A {
  public function foo() {
    echo 'foo called';
  }
}

class B extends A {
 }


<<__EntryPoint>>
function main_1201() {
fb_intercept('A::foo', function($_, $called_on) {
  var_dump($called_on);
}
);

A::foo();
B::foo();

// Trigger run_intercept_handler_for_invokefunc codepath
$class = 'B';
$c = 'call_user_fun';
$c .= 'c';
$c(array($class, 'foo'));
}
