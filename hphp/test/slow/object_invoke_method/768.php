<?php

// as a private/protected method
class C6 {
  private function __invoke($a0) {
    var_dump($a0);
  }
}
class C7 {
  protected function __invoke($a0) {
    var_dump($a0);
  }
}
$c = new C6;
$c(10);
 // still works...
$c = new C7;
$c(20);
 // still works...
