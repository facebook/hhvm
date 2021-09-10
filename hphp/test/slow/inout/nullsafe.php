<?hh

class A {
  public function foo(inout $a, inout $b, $c, inout $d) {
    $a = 1;
    $b = 2;
    $d = 3;
    return 4;
  }

  public function foo2(inout $a, inout $b, $c, inout $d) {
    throw new Exception("exception!");
  }

  public function foo3($a, $b, $c, $d) {
    return 123;
  }
};

function test1() {
  $a = 'a';
  $b = 'b';
  $c = 'c';
  $d = 'd';
  $e = null?->foo(inout $a, inout $b, $c, inout $d);
  var_dump($a, $b, $c, $d, $e);
}

function test2() {
  $a = 'a';
  $b = 'b';
  $c = 'c';
  $d = 'd';
  $e = null?->foo(inout $a, inout $b, inout $c, inout $d);
  var_dump($a, $b, $c, $d, $e);
}

function test3() {
  $o = new A();
  if (__hhvm_intrinsics\launder_value(true)) $o = null;
  $a = 'a';
  $b = 'b';
  $c = 'c';
  $d = 'd';
  $e = $o?->foo(inout $a, inout $b, $c, inout $d);
  var_dump($a, $b, $c, $d, $e);
}

function test4() {
  $o = new A();
  if (__hhvm_intrinsics\launder_value(true)) $o = null;
  $a = 'a';
  $b = 'b';
  $c = 'c';
  $d = 'd';
  $e = $o?->foo(inout $a, inout $b, inout $c, inout $d);
  var_dump($a, $b, $c, $d, $e);
}

function test5() {
  $o = new A();
  if (__hhvm_intrinsics\launder_value(true)) $o = null;
  $a = 'a';
  $b = 'b';
  $c = 'c';
  $d = 'd';
  $e = $o?->foo2(inout $a, inout $b, $c, inout $d);
  var_dump($a, $b, $c, $d, $e);
}

function test6() {
  $o = new A();
  if (__hhvm_intrinsics\launder_value(true)) $o = null;
  $a = 'a';
  $b = 'b';
  $c = 'c';
  $d = 'd';
  $e = $o?->foo3(inout $a, inout $b, $c, inout $d);
  var_dump($a, $b, $c, $d, $e);
}

<<__EntryPoint>>
function main() {
  test1();
  test2();
  test3();
  test4();
  test5();
  test6();
}
