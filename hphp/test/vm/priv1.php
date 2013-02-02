<?php
class C1 {
  private function foo() {
    echo "C1::foo\n";
  }
  public function test() {
    // This should call C1::foo, not D1::foo
    $obj = new D1;
    $obj->foo();
  }
}
class D1 extends C1 {
  private function foo() {
    echo "D1::foo\n";
  }
}
$obj = new C1;
$obj->test();



class C2 {
  private function foo() {
    echo "C2::foo\n";
  }
  public function test() {
    $this->foo();
  }
}
class D2 extends C2 {
  protected function foo() {
    echo "D2::foo\n";
  }
}
class E2 extends D2 {
}
$obj = new E2;
// This should call C2::foo, not D2::foo
$obj->test();



class C3 {
  private function foo() {
    echo "C3::foo\n";
  }
  public function test() {
    $this->foo();
  }
}
class D3 extends C3 {
  public function foo() {
    echo "D3::foo\n";
  }
}
$obj = new D3;
// This should call C3::foo, not D3::foo
$obj->test();



class A4 {
  private function foo() {
    echo "A4::foo\n";
  }
}
class B4 extends A4 {
  public function foo() {
    echo "B4::foo\n";
  }
}
class C4 {
  private function foo() {
    echo "C4::foo\n";
  }
  public static function test($obj) {
    $obj->foo();
  }
}
C4::test(new B4);


class A5 {
  private function __construct() {
  }
}
class B5 extends A5 {
  public static function test() {
    return new A5;
  }
}
B5::test();

