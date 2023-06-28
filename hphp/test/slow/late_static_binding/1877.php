<?hh

class A {
  static public function foo() :mixed{
    static::bar();
  }
  public static function bar() :mixed{
    var_dump(__CLASS__);
  }
  public function foo2() :mixed{
    B::foo();
    // B always changes 'static'
    self::foo();
 // 'self' doesn't change 'static'
  }
}
class B extends A {
  public static function bar() :mixed{
    var_dump(__CLASS__);
  }
  public function foo3() :mixed{
    $this::foo();
  // $this changes 'static'
    parent::foo();
 // 'parent' doesn't change 'static'
  }
}
 // BA


<<__EntryPoint>>
function main_1877() :mixed{
$a = new A();
$b = new B();

B::foo();
   // B

$b->foo2();
 // BB
$b->foo3();
 // BB

A::foo();
   // A

$a->foo2();
}
