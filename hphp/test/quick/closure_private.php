<?hh

class A {

  public function testPublic() :mixed{
    $a = function() {
      return $this->justReturn("foo");
    };
    return $a();
  }

  public function testUse() :mixed{
    $a = "foo";
    $b = function() use ($a) {
      return $this->justReturn($a);
    };
    return $b();
  }

  public function testParam() :mixed{
    $a = "foo";
    $b = function($foo) {
      return $this->justReturn($foo);
    };
    return $b($a);
  }

  public function testParamAndClosure() :mixed{
    $a = "foo";
    $b = "bar";
    $c = function($foo) use ($b) {
      return $this->justReturn($foo, $b);
    };
    return $c($a);
  }

  public function testNotByRef() :mixed{
    $a = "foo";
    $b = "bar";
    $c = function($foo) use ($b) {
      $this->double(inout $foo, inout $b);
    };
    $c($a);
    return $a.$b;
  }

  private function justReturn(...$args) :mixed{
    return $args;
  }

  private function double(inout $a, inout $b) :mixed{
    $a = $a.$a;
    $b = $b.$b;
  }

}
<<__EntryPoint>> function main(): void {
$a = new A;
var_dump($a->testPublic());
var_dump($a->testUse());
var_dump($a->testParam());
var_dump($a->testParamAndClosure());
var_dump($a->testNotByRef());
}
