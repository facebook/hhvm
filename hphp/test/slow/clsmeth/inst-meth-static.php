<?hh

abstract class C {
  protected int $x = 42;
  abstract function foo();
  function bar() { return class_meth(static::class, 'foo'); }
}

class D extends C {
  function foo() { return $this->x; }
}

<<__EntryPoint>>
function main() {
  $f = (new D)->bar();
  echo "FAIL\n";
  var_dump($f());
}
