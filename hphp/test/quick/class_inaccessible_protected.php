<?php
class someClass1 {
  protected static function someMethod() {
    //do things
  }
}

class someClass2 {
  public function __construct() {
    someClass1::someMethod();
  }
}

$someClass2 = new someClass2;
