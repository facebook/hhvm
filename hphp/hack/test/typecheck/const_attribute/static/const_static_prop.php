<?hh

class A {
  <<__Const>> static public int $static_const_prop = 5;
  const int CONST = 10;
  <<__Const>> public int $const_prop = 10;

  public function __construct(){
    $this::$static_const_prop = 50;  // illegal
    $this::CONST = 40;               // illegal
    $this->const_prop = 40;          // ok, not static elt
  }

  public function write_const():void{
    A::$static_const_prop = 10;      // illegal
    A::CONST = 50;                   // illegal
    $this->const_prop = 40;          // illegal
  }

  public function write_const_static():void{
    static::$static_const_prop = 50;
  }
}

function write_outside_class():void{
  A::$static_const_prop = 50;       // illegal
}
