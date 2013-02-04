<?php
class A {
  public function b() {
    return function() {
      return function() {
        return $this->c();
      };
    };
  }
  private function c() {
    return 91;
  }
}
$a = new A;
$b = $a->b();
$first = $b();
$second = $first();
var_dump($second);
