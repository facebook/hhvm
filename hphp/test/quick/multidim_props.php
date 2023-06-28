<?hh

function baz(inout $x) :mixed{
  var_dump($x);
}

class A {
  private $x;

  function foo() :mixed{
    $this->x = $this;
  }
  function bar() :mixed{
    try {
      $y = $this->x->x;
      baz(inout $y);
    } catch (UndefinedPropertyException $e) {
      var_dump($e->getMessage());
    }
  }

  public function what() :mixed{
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
