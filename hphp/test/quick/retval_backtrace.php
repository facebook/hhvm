<?php

class Bro {}

class Thing1 {
  public function __destruct() {
    $z = debug_backtrace();
    //var_dump($z);
  }
}

class Thing2 {
  public function foo() {
    $a = new Bro();
    $ab = new Bro();
    $abc = new Bro();
    $abcd = new Bro();
    $abcde = new Bro();
    $abcdef = new Bro();
    $z = new Thing1();
    return 12;
  }
}

function foo() {
  echo "go\n";
  $z = new Thing2();
  return $z->foo();
}

echo "Returned: " . foo() . "\n";