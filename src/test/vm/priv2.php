<?php
class C {
  private function foo() {
    echo "C::foo\n";
  }
  public function test() {
    // This should not call C::foo, it should fatal
    D::foo();
  }
}
class D extends C {
  private function foo() {
    echo "D::foo\n";
  }
}
$obj = new C;
$obj->test();
