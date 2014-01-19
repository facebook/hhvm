<?php

class Exception1 extends Exception {
  public function __Construct() {
    parent::__construct();
  }
}

class Exception2 extends Exception1 {
  public function exception2() {
    parent::__construct();
  }
}

function foo() {
  throw new Exception2();
}

function bar() {
  try {
    foo();
  }
 catch (Exception $exn) {
    $a = $exn->getTrace();
 foreach ($a as &$b) $b['file'] = 'string';
    var_dump($a);
    var_dump($exn->getLine());
  }
}

bar();
