<?php

/**
 * For some of the tests below the VM produces different output than
 * Zend. This is because Zend is more liberal about passing in the
 * current instance to the callee. Zend is in the process of deprecating
 * this behavior.
 */

class D1 {
  private function foo() {
    echo "D1::foo " . (isset($this) ? "true\n" : "false\n");
  }
  public function test() {
    D1::foo();
  }
}
class E1 extends D1 {
}
$e1 = new E1;
$e1->test(); // Outputs 'D1:foo true'

class D2 {
  private function foo() {
    echo "D2::foo " . (isset($this) ? "true\n" : "false\n");
  }
  public function test() {
    F2::foo();
  }
}
class E2 extends D2 {
}
class F2 extends D2 {
}
$e2 = new E2;
$e2->test(); // Outputs 'D2::foo false' (Zend outputs 'D2::foo true')

class D3 {
  private function foo() {
    echo "D3::foo " . (isset($this) ? "true\n" : "false\n");
  }
  public function test() {
    F3::foo();
  }
}
class X3 extends D3 {
  private function foo() {
    echo "X3::foo\n";
  }
}
class E3 extends X3 {
}
class F3 extends D3 {
}
$e3 = new E3;
$e3->test(); // Outputs 'D3::foo false' (Zend outputs 'D3::foo true')

class D4 {
  private function foo() {
    echo "D4::foo " . (isset($this) ? "true\n" : "false\n");
  }
  public static function test() {
    F4::foo();
  }
}
class F4 extends D4 {
}
$e4 = new F4;
$e4->test(); // Outputs 'D4::foo false'

class D5 {
  private function foo() {
    echo "D5::foo " . (isset($this) ? "true\n" : "false\n");
  }
  public function test() {
    F5::foo();
  }
}
class E5 extends D5 {
}
class X5 extends D5 {
  private function foo() {
    echo "X5::foo\n";
  }
}
class F5 extends X5 {
}
$e5 = new E5;
$e5->test(); // Fatals

