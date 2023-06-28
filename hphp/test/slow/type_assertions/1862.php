<?hh

class X {
  public $propX;
  function baz() :mixed{
    echo 'X';
  }
  function foo() :mixed{
    if ($this is Y) {
      $this->bar();
      $this->baz();
      var_dump($this->propX);
      return $this->propY;
    }
    return null;
  }
}
class Y extends X {
  public $propY;
  function bar():mixed{
    echo 'Y';
  }
}
class A1 {
  public $a1prop;
  function a1method() :mixed{
    return 0;
  }
  function doStuff() :mixed{
    if ($this is D1) {
      var_dump($this->d1prop);
      var_dump($this->d1method());
    }
    else if ($this is C1) {
      var_dump($this->c1prop);
      var_dump($this->c1method());
    }
    else if ($this is B1) {
      var_dump($this->b1prop);
      var_dump($this->b1method());
    }
    else if ($this is A1) {
      var_dump($this->a1prop);
      var_dump($this->a1method());
    }
  }
}
class B1 extends A1 {
  public $b1prop;
  function b1method() :mixed{
    return 1;
  }
}
<<__EntryPoint>>
function entrypoint_1862(): void {
  $y = new Y;
  $y->propX = 16;
  $y->propY = 32;
  var_dump($y->foo());
  if (rand(0, 1)) {
    include '1862-1.inc';
  } else {
    include '1862-2.inc';
  }
  include '1862-after.inc';
  $a1 = new A1;
  $a1->a1prop = 0;
  $a1->doStuff();
  $b1 = new B1;
  $b1->b1prop = 1;
  $b1->doStuff();
  $c1 = new C1;
  $c1->c1prop = 2;
  $c1->doStuff();
  $d1 = new D1;
  $d1->d1prop = 3;
  $d1->doStuff();
}
