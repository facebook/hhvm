<?php

class CGetMTest {
  public function __construct() {
    $this->uniqueVar = "a string";
  }

  public function getX() {
    // TODO test something that will throw, make sure stack
    // materialization worked.
    return $this->uniqueVar;
  }

  private $uniqueVar;
}

function test5() {
  $obj = new CGetMTest();
  echo $obj->getX();
  echo "\n";
}

function test9() {
  // $this is on the stack; can we still handle incref / decref
  // elimination (not right now).
  echo (new CGetMTest())->getX();
  echo "\n";
}

test5();
test9();
