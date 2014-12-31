<?php
class someClass1 {
  private function someMethod() {
    //do things
  }
}

class someClass2 extends someClass1 {
  public function __construct() {
    $this->someMethod();
  }
}

$someClass2 = new someClass2;
