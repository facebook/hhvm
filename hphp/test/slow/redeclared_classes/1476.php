<?hh

class PEAR {
  static function f() :mixed{
    PEAR::g();
  }
  static function g() :mixed{
    echo 'This is g()';
  }
}
class D1 extends PEAR {
  public $foo;
  private $bar;
  function bar() :mixed{
    return $this->foo + $this->bar;
  }
}
class D2 extends D1 {
  public $foo;
  private $bar;
  function bar() :mixed{
    return $this->foo + $this->bar;
  }
}
<<__EntryPoint>>
function entrypoint_1476(): void {
  PEAR::f();
}
