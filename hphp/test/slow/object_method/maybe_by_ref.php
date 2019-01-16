<?hh

class X {
  private $x;

  function foo() {
    $thiz = &$this;
    $this->bar(&$thiz);
    return $this;
  }

  function bar($x) {}
}

class Y extends X {
  function bar(&$x) { $x = 42; }
}


<<__EntryPoint>>
function main_maybe_by_ref() {
var_dump((new Y())->foo());
}
