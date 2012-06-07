<?php
class A1 {
  private function bar() { echo "A1::bar\n"; }
}
class B1 extends A1 {
  protected function bar() { echo "B1::bar\n"; }
  public function baz() { $this->bar(); }
}
class C1 extends B1 {
  protected function bar() { echo "C1::bar\n"; }
}
$obj = new C1;
$obj->baz();

class A2 {
  private function bar() { echo "A2::bar\n"; }
}
class B2 extends A2 {
  public function bar() { echo "B2::bar\n"; }
  public function baz() { $this->bar(); }
}
class C2 extends B2 {
  public function bar() { echo "C2::bar\n"; }
}
$obj = new C2;
$obj->baz();

class A3 {
  private function bar() { echo "A3::bar\n"; }
}
class B3 extends A3 {
  protected function bar() { echo "B3::bar\n"; }
  public function baz() { $this->bar(); }
}
class C3 extends B3 {
  public function bar() { echo "C3::bar\n"; }
}
$obj = new C3;
$obj->baz();

class A4 {
  private function bar() { echo "A4::bar\n"; }
}
class B4 extends A4 {
  public function baz() { $this->bar(); }
}
$obj = new B4;
$obj->baz();
