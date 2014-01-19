<?php

class MyException extends Exception {
  public function __construct() {
  }
}
function thrower() {
  throw new MyException();
}
try {
  thrower();
}
 catch (Exception $exn) {
  $a = $exn->getTrace();
 foreach ($a as &$b) $b['file'] = 'string';
  var_dump($a);
  var_dump($exn->getLine());
}
