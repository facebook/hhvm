<?php

include __DIR__."/builtin_extensions.inc";

class A_StringBuffer extends DateInterval {
  public $___x;
}
test("StringBuffer");

function main() {
  echo "================\n";

  $x = new StringBuffer();
  $x->foo = 123;
  var_dump($x);
  var_dump($x->foo);

  echo "================\n";

  class Foo extends StringBuffer {
    public $foo = 42;
    public $bar = 'seven';
  }
  $y = new Foo;
  $y->biz = array();
  var_dump($y);
  var_dump($y->foo);
  var_dump($y->bar);
  var_dump($y->biz);
}

main();
