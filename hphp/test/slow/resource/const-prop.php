<?
class Test {
  private $foo = STDOUT;
  function bar() { fprintf($this->foo, "Baz\n"); }
}

(new Test)->bar();
