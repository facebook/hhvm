<?hh

  class A {
    protected $a = 1;
  }
  class B extends A {
    public $a;
    function f() {
 $this->a = 2;
 }
  }

  $x = 1;
