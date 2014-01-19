<?php

$b = 123;
if ($b) {
  class Exception1 extends Exception {
}
}
 else {
  class Exception1 extends Exception {
}
}
class Exception2 extends Exception1 {
}

function foo() {
  $e = new Exception();
  try {
    throw new Exception2();
  }
 catch (Exception $e) {
    var_dump($e->getCode());
  }
}
foo();
