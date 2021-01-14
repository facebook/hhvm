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
    try {
      $y = $this->x->x;
      baz(inout $y);
    } catch (UndefinedPropertyException $e) {
      var_dump($e->getMessage());
    }
  }

  public function what() {
    $this->foo();
    $this->bar();
    unset($this->x->x);
    $this->bar();
  }
}

<<__EntryPoint>>
function foo(): void {
  $x = new A();
  $x->what();
  $x->what();
}
