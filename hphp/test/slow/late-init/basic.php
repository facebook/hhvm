<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  <<__LateInit>> public $p1;
  <<__LateInit>> private $p2;
  <<__LateInit>> public static $p3;
  <<__LateInit>> private static $p4;

  <<__LateInit>> public static $p5;
  <<__LateInit>> private static $p6;

  <<__LateInit>> public static $p7;
  <<__LateInit>> private static $p8;
  <<__LateInit>> private $p9;
  <<__LateInit>> private $p10;

  <<__LateInit>> public $p11;
  <<__LateInit>> private $p12;
  <<__LateInit>> public static $p13;
  <<__LateInit>> private static $p14;

  public function set() {
    $this->p1 = 123;
    $this->p2 = 123;
    A::$p3 = 123;
    A::$p4 = 123;
    A::$p5 = new A();
    A::$p6 = new A();
    A::$p7 = new A();
    A::$p7->p9 = new A();
    A::$p7->p9->p10 = 123;
    A::$p8 = new A();
    A::$p8->p9 = new A();
    A::$p8->p9->p10 = 123;
  }

  public function unset() {
    unset($this->p1);
    unset($this->p2);
    unset(A::$p7->p9->p10);
    unset(A::$p8->p9->p10);
  }

  public function get1() { return $this->p1; }
  public function isset1() { return isset($this->p1); }
  public function empty1() { return empty($this->p1); }
  public function incdec1() { return ++$this->p1; }
  public function setop1() { return $this->p1 += 1000; }
  public function bind1() {
    $b = 123;
    $this->p1 =& $b;
    return $this->p1;
  }
  public function vget1() {
    $x =& $this->p1;
    return $x;
  }

  public function get2() { return $this->p2; }
  public function isset2() { return isset($this->p2); }
  public function empty2() { return empty($this->p2); }
  public function incdec2() { return ++$this->p2; }
  public function setop2() { return $this->p2 += 1000; }
  public function bind2() {
    $b = 123;
    $this->p2 =& $b;
    return $this->p2;
  }
  public function vget2() {
    $x =& $this->p2;
    return $x;
  }

  public function get3() { return A::$p3; }
  public function isset3() { return isset(A::$p3); }
  public function empty3() { return empty(A::$p3); }
  public function incdec3() { return ++A::$p3; }
  public function setop3() { return A::$p3 += 1000; }
  public function bind3() {
    $b = 123;
    A::$p3 =& $b;
    return A::$p3;
  }
  public function vget3() {
    $x =& A::$p3;
    return $x;
  }

  public function get4() { return A::$p4; }
  public function isset4() { return isset(A::$p4); }
  public function empty4() { return empty(A::$p4); }
  public function incdec4() { return ++A::$p4; }
  public function setop4() { return A::$p4 += 1000; }
  public function bind4() {
    $b = 123;
    A::$p4 =& $b;
    return A::$p4;
  }
  public function vget4() {
    $x =& A::$p4;
    return $x;
  }

  public function get5() { return A::$p5->p9->p10; }
  public function get6() { return A::$p6->p9->p10; }
  public function get7() { return A::$p7->p9->p10; }
  public function get8() { return A::$p8->p9->p10; }

  public function get9() { return $this->p11; }
  public function get10() { return $this->p12; }
  public function get11() { return A::$p13; }
  public function get12() { return A::$p14; }

  public function isset5() { return isset($this->p11); }
  public function isset6() { return isset($this->p12); }
  public function isset7() { return isset(A::$p13); }
  public function isset8() { return isset(A::$p14); }

  public function empty5() { return empty($this->p11); }
  public function empty6() { return empty($this->p12); }
  public function empty7() { return empty(A::$p13); }
  public function empty8() { return empty(A::$p14); }
}

class B {
  <<__LateInit>> public $p1;
  <<__LateInit>> private $p2;
  <<__LateInit>> public static $p3;
  <<__LateInit>> private static $p4;

  <<__LateInit>> public static $p5;
  <<__LateInit>> private static $p6;

  <<__LateInit>> public static $p7;
  <<__LateInit>> private static $p8;
  <<__LateInit>> private $p9;
  <<__LateInit>> private $p10;

  <<__LateInit>> public $p11;
  <<__LateInit>> private $p12;
  <<__LateInit>> public static $p13;
  <<__LateInit>> private static $p14;

  private static function prop($i) {
    return __hhvm_intrinsics\launder_value("p$i");
  }

  public function set() {
    $this->{B::prop(1)} = 123;
    $this->{B::prop(2)} = 123;
    B::$p3 = 123;
    B::$p4 = 123;
    B::$p5 = new B();
    B::$p6 = new B();
    B::$p7 = new B();
    B::$p7->{B::prop(9)} = new B();
    B::$p7->{B::prop(9)}->{B::prop(10)} = 123;
    B::$p8 = new B();
    B::$p8->{B::prop(9)} = new B();
    B::$p8->{B::prop(9)}->{B::prop(10)} = 123;
  }

  public function unset() {
    unset($this->{B::prop(1)});
    unset($this->{B::prop(2)});
    unset(B::$p7->{B::prop(9)}->{B::prop(10)});
    unset(B::$p8->{B::prop(9)}->{B::prop(10)});
  }

  public function get1() { return $this->{B::prop(1)}; }
  public function isset1() { return isset($this->{B::prop(1)}); }
  public function empty1() { return empty($this->{B::prop(1)}); }
  public function incdec1() { return ++$this->{B::prop(1)}; }
  public function setop1() { return $this->{B::prop(1)} += 1000; }
  public function bind1() {
    $b = 123;
    $this->{B::prop(1)} =& $b;
    return $this->{B::prop(1)};
  }
  public function vget1() {
    $x =& $this->{B::prop(1)};
    return $x;
  }

  public function get2() { return $this->{B::prop(2)}; }
  public function isset2() { return isset($this->{B::prop(2)}); }
  public function empty2() { return empty($this->{B::prop(2)}); }
  public function incdec2() { return ++$this->{B::prop(2)}; }
  public function setop2() { return $this->{B::prop(2)} += 1000; }
  public function bind2() {
    $b = 123;
    $this->{B::prop(2)} =& $b;
    return $this->{B::prop(2)};
  }
  public function vget2() {
    $x =& $this->{B::prop(2)};
    return $x;
  }

  public function get3() { return B::$p3; }
  public function isset3() { return isset(B::$p3); }
  public function empty3() { return empty(B::$p3); }
  public function incdec3() { return ++B::$p3; }
  public function setop3() { return B::$p3 += 1000; }
  public function bind3() {
    $b = 123;
    B::$p3 =& $b;
    return B::$p3;
  }
  public function vget3() {
    $x =& B::$p3;
    return $x;
  }

  public function get4() { return B::$p4; }
  public function isset4() { return isset(B::$p4); }
  public function empty4() { return empty(B::$p4); }
  public function incdec4() { return ++B::$p4; }
  public function setop4() { return B::$p4 += 1000; }
  public function bind4() {
    $b = 123;
    B::$p4 =& $b;
    return B::$p4;
  }
  public function vget4() {
    $x =& B::$p4;
    return $x;
  }

  public function get5() { return B::$p5->{B::prop(9)}->{B::prop(10)}; }
  public function get6() { return B::$p6->{B::prop(9)}->{B::prop(10)}; }
  public function get7() { return B::$p7->{B::prop(9)}->{B::prop(10)}; }
  public function get8() { return B::$p8->{B::prop(9)}->{B::prop(10)}; }

  public function get9() { return $this->{B::prop(11)}; }
  public function get10() { return $this->{B::prop(12)}; }
  public function get11() { return B::$p13; }
  public function get12() { return B::$p14; }

  public function isset5() { return isset($this->{B::prop(11)}); }
  public function isset6() { return isset($this->{B::prop(12)}); }
  public function isset7() { return isset(B::$p13); }
  public function isset8() { return isset(B::$p14); }

  public function empty5() { return empty($this->{B::prop(11)}); }
  public function empty6() { return empty($this->{B::prop(12)}); }
  public function empty7() { return empty(B::$p13); }
  public function empty8() { return empty(B::$p14); }
}

const TESTS = vec[
  'get1',
  'isset1',
  'empty1',
  'incdec1',
  'setop1',
  'bind1',
  'vget1',
  'get2',
  'isset2',
  'empty2',
  'incdec2',
  'setop2',
  'bind2',
  'vget2',
  'get3',
  'isset3',
  'empty3',
  'incdec3',
  'setop3',
  'bind3',
  'vget3',
  'get4',
  'isset4',
  'empty4',
  'incdec4',
  'setop4',
  'bind4',
  'vget4',
  'get5',
  'get6',
  'get7',
  'get8',
  'get9',
  'get10',
  'get11',
  'get12',
  'isset5',
  'isset6',
  'isset7',
  'isset8',
  'empty5',
  'empty6',
  'empty7',
  'empty8',
];

function run_test($a, $test) {
  try {
    echo "=== $test ===\n";
    var_dump($a->$test());
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
}

function test($tests, $a) {
  foreach ($tests as $test) run_test($a, $test);
  echo "============= setting ================\n";
  $a->set();
  foreach ($tests as $test) run_test($a, $test);
  echo "============= unsetting ==============\n";
  $a->unset();
  $a->unset();
  foreach ($tests as $test) run_test($a, $test);
}
test(TESTS, new A());
test(TESTS, new B());
