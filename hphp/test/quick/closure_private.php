<?php

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

  public function testByRef() {
    $a = "foo";
    $b = "bar";
    $c = function(&$foo) use (&$b) {
      $this->double($foo, $b);
    };
    $c($a);
    return $a.$b;
  }

  public function testNotByRef() {
    $a = "foo";
    $b = "bar";
    $c = function($foo) use ($b) {
      $this->double($foo, $b);
    };
    $c($a);
    return $a.$b;
  }

  private function justReturn() {
    return func_get_args();
  }

  private function double(&$a, &$b) {
    $a = $a.$a;
    $b = $b.$b;
  }

}
$a = new A;
var_dump($a->testPublic());
var_dump($a->testUse());
var_dump($a->testParam());
var_dump($a->testParamAndClosure());
var_dump($a->testByRef());
var_dump($a->testNotByRef());
