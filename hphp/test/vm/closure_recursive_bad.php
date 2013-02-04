<?php
class A {
  public function b() {
    function c() {
      return function() {
        return $this->d();
      };
    };
    return c();
  }
  private function d() {
    return 91;
  }
}
$a = new A;
$b = $a->b();
var_dump($b());
