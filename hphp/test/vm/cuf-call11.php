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
    call_user_func('D1::foo');
    call_user_func(array('D1','foo'));
  }
}
class E1 extends D1 {
}

function main1() {
  $e1 = new E1;
  $e1->test(); // Outputs 'D1:foo true'
}
main1();

class D2 {
  private function foo() {
    echo "D2::foo " . (isset($this) ? "true\n" : "false\n");
  }
  public function test() {
    call_user_func('F2::foo');
    call_user_func(array('F2','foo'));
  }
}
class E2 extends D2 {
}
class F2 extends D2 {
}

function main2() {
$e2 = new E2;
$e2->test(); // Outputs 'D2::foo false' (Zend outputs 'D2::foo true')
}
main2();


class D3 {
  private function foo() {
    echo "D3::foo " . (isset($this) ? "true\n" : "false\n");
  }
  public function test() {
    call_user_func('F3::foo');
    call_user_func(array('F3','foo'));
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

function main3() {
$e3 = new E3;
$e3->test(); // Outputs 'D3::foo false' (Zend outputs 'D3::foo true')
}
main3();

class D4 {
  private function foo() {
    echo "D4::foo " . (isset($this) ? "true\n" : "false\n");
  }
  public static function test() {
    call_user_func('F4::foo');
    call_user_func(array('F4','foo'));
  }
}
class F4 extends D4 {
}

function main4() {
  $e4 = new F4;
  $e4->test(); // Outputs 'D4::foo false'
}
main4();

class D5 {
  private function foo() {
    echo "D5::foo " . (isset($this) ? "true\n" : "false\n");
  }
  public function test() {
    call_user_func('F5::foo');
    call_user_func(array('F5','foo'));
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

function main5() {
  $e5 = new E5;
  $e5->test(); // Fatals
}
main5();

