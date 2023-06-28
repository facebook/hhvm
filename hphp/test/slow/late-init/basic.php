<?hh



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

  public function set() :mixed{
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

  public function unset() :mixed{
    unset($this->p1);
    unset($this->p2);
    unset(A::$p7->p9->p10);
    unset(A::$p8->p9->p10);
  }

  public function get1() :mixed{ return $this->p1; }
  public function isset1() :mixed{ return isset($this->p1); }
  public function incdec1() :mixed{ return ++$this->p1; }
  public function setop1() :mixed{ return $this->p1 += 1000; }


  public function get2() :mixed{ return $this->p2; }
  public function isset2() :mixed{ return isset($this->p2); }
  public function incdec2() :mixed{ return ++$this->p2; }
  public function setop2() :mixed{ return $this->p2 += 1000; }


  public function get3() :mixed{ return A::$p3; }
  public function isset3() :mixed{ return isset(A::$p3); }
  public function incdec3() :mixed{ return ++A::$p3; }
  public function setop3() :mixed{ return A::$p3 += 1000; }

  public function get4() :mixed{ return A::$p4; }
  public function isset4() :mixed{ return isset(A::$p4); }
  public function incdec4() :mixed{ return ++A::$p4; }
  public function setop4() :mixed{ return A::$p4 += 1000; }

  public function get5() :mixed{ return A::$p5->p9->p10; }
  public function get6() :mixed{ return A::$p6->p9->p10; }
  public function get7() :mixed{ return A::$p7->p9->p10; }
  public function get8() :mixed{ return A::$p8->p9->p10; }

  public function get9() :mixed{ return $this->p11; }
  public function get10() :mixed{ return $this->p12; }
  public function get11() :mixed{ return A::$p13; }
  public function get12() :mixed{ return A::$p14; }

  public function isset5() :mixed{ return isset($this->p11); }
  public function isset6() :mixed{ return isset($this->p12); }
  public function isset7() :mixed{ return isset(A::$p13); }
  public function isset8() :mixed{ return isset(A::$p14); }
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

  private static function prop($i) :mixed{
    return __hhvm_intrinsics\launder_value("p$i");
  }

  public function set() :mixed{
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

  public function unset() :mixed{
    unset($this->{B::prop(1)});
    unset($this->{B::prop(2)});
    unset(B::$p7->{B::prop(9)}->{B::prop(10)});
    unset(B::$p8->{B::prop(9)}->{B::prop(10)});
  }

  public function get1() :mixed{ return $this->{B::prop(1)}; }
  public function isset1() :mixed{ return isset($this->{B::prop(1)}); }
  public function incdec1() :mixed{ return ++$this->{B::prop(1)}; }
  public function setop1() :mixed{ return $this->{B::prop(1)} += 1000; }


  public function get2() :mixed{ return $this->{B::prop(2)}; }
  public function isset2() :mixed{ return isset($this->{B::prop(2)}); }
  public function incdec2() :mixed{ return ++$this->{B::prop(2)}; }
  public function setop2() :mixed{ return $this->{B::prop(2)} += 1000; }


  public function get3() :mixed{ return B::$p3; }
  public function isset3() :mixed{ return isset(B::$p3); }
  public function incdec3() :mixed{ return ++B::$p3; }
  public function setop3() :mixed{ return B::$p3 += 1000; }

  public function get4() :mixed{ return B::$p4; }
  public function isset4() :mixed{ return isset(B::$p4); }
  public function incdec4() :mixed{ return ++B::$p4; }
  public function setop4() :mixed{ return B::$p4 += 1000; }

  public function get5() :mixed{ return B::$p5->{B::prop(9)}->{B::prop(10)}; }
  public function get6() :mixed{ return B::$p6->{B::prop(9)}->{B::prop(10)}; }
  public function get7() :mixed{ return B::$p7->{B::prop(9)}->{B::prop(10)}; }
  public function get8() :mixed{ return B::$p8->{B::prop(9)}->{B::prop(10)}; }

  public function get9() :mixed{ return $this->{B::prop(11)}; }
  public function get10() :mixed{ return $this->{B::prop(12)}; }
  public function get11() :mixed{ return B::$p13; }
  public function get12() :mixed{ return B::$p14; }

  public function isset5() :mixed{ return isset($this->{B::prop(11)}); }
  public function isset6() :mixed{ return isset($this->{B::prop(12)}); }
  public function isset7() :mixed{ return isset(B::$p13); }
  public function isset8() :mixed{ return isset(B::$p14); }
}

const TESTS = vec[
  'get1',
  'isset1',
  'incdec1',
  'setop1',

  'get2',
  'isset2',
  'incdec2',
  'setop2',

  'get3',
  'isset3',
  'incdec3',
  'setop3',
  'get4',
  'isset4',
  'incdec4',
  'setop4',
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
];

function run_test($a, $test) :mixed{
  try {
    echo "=== $test ===\n";
    var_dump($a->$test());
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
}

function test($tests, $a) :mixed{
  foreach ($tests as $test) run_test($a, $test);
  echo "============= setting ================\n";
  $a->set();
  foreach ($tests as $test) run_test($a, $test);
  echo "============= unsetting ==============\n";
  $a->unset();
  $a->unset();
  foreach ($tests as $test) run_test($a, $test);
}
<<__EntryPoint>>
function main_entry(): void {
  test(TESTS, new A());
  test(TESTS, new B());
}
