<?hh

class foo {
  private $test = 3;

  public function x($fn) :mixed{
    $this->a = $fn;
    var_dump($this->a->__invoke());
    var_dump(is_a($this->a, 'Closure'));
    var_dump(is_callable($this->a));

    return $this->a;
  }
}
<<__EntryPoint>> function main(): void {
$foo = new foo;
$y = $foo->x(function() use ($foo) { return $foo; });
var_dump($y()->test);
}
