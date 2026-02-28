<?hh

class A {
  private $pri = 'a-pri';
  protected $pro = 'a-pro';
  function bar() :mixed{
    var_dump($this->pri);
  }
  function bar2() :mixed{
    var_dump($this->pro);
  }
}
class B extends A {
  private $pri = 'b-pri';
  function bar() :mixed{
    parent::bar();
    var_dump($this->pri);
  }
}
class C extends B {
  public $pri = 'c-pri';
  public $pro = 'c-pro';
  function bar2() :mixed{
    parent::bar2();
    var_dump($this->pro);
  }
}
class Base {
  protected $pro = 1;
  private $pri = 'base-pri';
  function q0() :mixed{
    var_dump($this->pri);
  }
}
class R extends Base {
  public $rpub = 1;
  protected $pro = 2;
  private $pri = 'r-pri';
  function q() :mixed{
    var_dump($this->pri);
  }
}
class D extends R {
  public $dpub = 'd';
  protected $pro = 'pro';
  private $pri = 'd-pri';
  function qq() :mixed{
    var_dump($this->pri);
  }
}
class DD extends D {
  private $pri = 'dd-pri';
  function qqq() :mixed{
    var_dump($this->pri);
  }
}

<<__EntryPoint>>
function main_686() :mixed{
  $obj = new B();
  $obj->bar();
  $obj = new C;
  $obj->bar();
  $obj->bar2();
  var_dump(serialize($obj));
  if (__hhvm_intrinsics\launder_value(false)) {
    include '686.inc';
  }
  $d = new D;
  $d->qq();
  $d->q();
  $d->q0();
  $d = new DD;
  $d->qqq();
  $d->qq();
  $d->q();
  $d->q0();
}
