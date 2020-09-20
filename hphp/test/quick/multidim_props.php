<?hh

function baz(inout $x) {
  var_dump($x);
}

class A {
  private $x;

  function foo() {
    $this->x = $this;
  }
  function bar() {
    $y = $this->x->x->x->x->x;
    baz(inout $y);
    $y = $this->x->x->x->x->x;
    baz(inout $y);

  }

  public function what() {
    $this->foo();
    $this->bar();
    unset($this->x->x);
    $this->bar();
  }
}

<<__EntryPoint>> function foo(): void {
  $x = new A();
  $x->what();
  $x->what();
}
