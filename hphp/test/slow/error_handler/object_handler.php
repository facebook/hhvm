<?php
class MyClass {
  public function __construct() {
    // See GitHub #2113
    $this->prop1 = 'something';
    $this->prop2 = 'somethingElse';
    $this->prop3 = 'somethingElseThen';

    var_dump(set_error_handler(array($this, 'errorHandler')));
  }

  public function errorHandler($severity, $message, $file = NULL,
                               $line = NULL) {
    return false;
  }

  public function fail() {
    user_error('Try to cause an error', E_USER_ERROR);
  }
}

function main() {
  $x = new MyClass();
  $x->fail();
}

main();
