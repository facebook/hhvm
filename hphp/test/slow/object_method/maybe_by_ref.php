<?hh

class X {
  private $x;

  function foo() {
    $this->bar($this);
    return $this;
  }

  function bar($x) {}
}

class Y extends X {
  function bar(&$x) { $x = 42; }
}

var_dump((new Y())->foo());
