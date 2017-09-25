<?hh

class foo {
  private $test = 3;

  public function x($fn) {
    $a = &$this;
    $this->a = $fn;
    var_dump($this->a->__invoke());
    var_dump(is_a($this->a, 'closure'));
    var_dump(is_callable($this->a));

    return $this->a;
  }
}

$foo = new foo;
$y = $foo->x(function() use (&$foo) { return $foo; });
var_dump($y()->test);
