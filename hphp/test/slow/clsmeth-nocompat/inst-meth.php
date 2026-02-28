<?hh

class C {
  private int $x = 42;
  function foo() :mixed{ return $this->x; }
}

class D extends C {
  function bar() :mixed{ return D::foo<>; }
}

<<__EntryPoint>>
function main() :mixed{
  $f = (new D)->bar();
  echo "FAIL\n";
  var_dump($f());
}
