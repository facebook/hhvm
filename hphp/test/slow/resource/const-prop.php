<?hh
class Test {
  const FOO_STDOUT = STDOUT;
  private $foo = STDOUT;
  function bar() { fprintf($this->foo, "Baz\n"); }
}

class Test2 extends Test {
  private $foo2 = Test::FOO_STDOUT;
}


<<__EntryPoint>>
function main_const_prop() {
(new Test)->bar();
(new Test2)->bar();
}
