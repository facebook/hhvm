<?hh

class A {

  public function testPublic() {
    $a = function() {
      return $this->justReturn("foo");
    };
    return $a();
  }

  public function testUse() {
    $a = "foo";
    $b = function() use ($a) {
      return $this->justReturn($a);
    };
    return $b();
  }

  public function testParam() {
    $a = "foo";
    $b = function($foo) {
      return $this->justReturn($foo);
    };
    return $b($a);
  }

  public function testParamAndClosure() {
    $a = "foo";
    $b = "bar";
    $c = function($foo) use ($b) {
      return $this->justReturn($foo, $b);
    };
    return $c($a);
  }

  public function testNotByRef() {
    $a = "foo";
    $b = "bar";
    $c = function($foo) use ($b) {
      $this->double(inout $foo, inout $b);
    };
    $c($a);
    return $a.$b;
  }

  private function justReturn(...$args) {
    return $args;
  }

  private function double(inout $a, inout $b) {
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
