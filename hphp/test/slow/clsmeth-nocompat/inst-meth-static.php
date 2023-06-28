<?hh

abstract class C {
  protected int $x = 42;
  abstract function foo():mixed;
  function bar() :mixed{ return static::foo<>; }
}

class D extends C {
  function foo() :mixed{ return $this->x; }
}

<<__EntryPoint>>
function main() :mixed{
  $f = (new D)->bar();
  echo "FAIL\n";
  var_dump($f());
}
