<?
class Test {
  const FOO_STDOUT = STDOUT;
  private $foo = STDOUT;
  function bar() { fprintf($this->foo, "Baz\n"); }
}

class Test2 extends Test {
  private $foo2 = Test::FOO_STDOUT;
}

(new Test)->bar();
(new Test2)->bar();
