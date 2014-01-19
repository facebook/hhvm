<?php

trait TraitFoo {
  public function getStringThroughProtectedMethod() {
    return $this->protectedMethod();
  }
  protected function protectedMethod() {
    return 'fallback protected method';
  }
  public function getStringThroughPrivateMethod() {
    return $this->privateMethod();
  }
  private function privateMethod() {
    return 'fallback private method';
  }
}
class A {
  use TraitFoo;
  protected function protectedMethod() {
    return 'in a protectedMethod';
  }
  private function privateMethod() {
    return 'in a privateMethod';
  }
}
$a = new A();
echo $a->getStringThroughProtectedMethod()."\n";
echo $a->getStringThroughPrivateMethod()."\n";
