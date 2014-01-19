<?php
interface I {
  public function foo();
}
class C implements I {
  protected function foo() { echo "foo\n"; }
  public function bar() { $this->foo(); }
}
$obj = new C;
$obj->bar();
