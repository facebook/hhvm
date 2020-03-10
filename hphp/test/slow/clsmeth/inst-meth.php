<?hh

class C {
  private int $x = 42;
  function foo() { return $this->x; }
}

class D extends C {
  function bar() { return class_meth('D', 'foo'); }
}

<<__EntryPoint>>
function main() {
  $f = (new D)->bar();
  echo "FAIL\n";
  var_dump($f());
}
