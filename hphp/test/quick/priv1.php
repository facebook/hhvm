<?hh
class C1 {
  private function foo() :mixed{
    echo "C1::foo\n";
  }
  public function test() :mixed{
    // This should call C1::foo, not D1::foo
    $obj = new D1;
    $obj->foo();
  }
}
class D1 extends C1 {
  private function foo() :mixed{
    echo "D1::foo\n";
  }
}

class C2 {
  private function foo() :mixed{
    echo "C2::foo\n";
  }
  public function test() :mixed{
    $this->foo();
  }
}
class D2 extends C2 {
  protected function foo() :mixed{
    echo "D2::foo\n";
  }
}
class E2 extends D2 {
}

class C3 {
  private function foo() :mixed{
    echo "C3::foo\n";
  }
  public function test() :mixed{
    $this->foo();
  }
}
class D3 extends C3 {
  public function foo() :mixed{
    echo "D3::foo\n";
  }
}

class A4 {
  private function foo() :mixed{
    echo "A4::foo\n";
  }
}
class B4 extends A4 {
  public function foo() :mixed{
    echo "B4::foo\n";
  }
}
class C4 {
  private function foo() :mixed{
    echo "C4::foo\n";
  }
  public static function test($obj) :mixed{
    $obj->foo();
  }
}

class A5 {
  private function __construct() {
  }
}
class B5 extends A5 {
  public static function test() :mixed{
    return new A5;
  }
}

<<__EntryPoint>> function main(): void {
$obj = new C1;
$obj->test();

$obj = new E2;
// This should call C2::foo, not D2::foo
$obj->test();

$obj = new D3;
// This should call C3::foo, not D3::foo
$obj->test();

C4::test(new B4);

B5::test();
}
