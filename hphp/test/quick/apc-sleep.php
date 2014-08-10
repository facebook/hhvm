<?php
class Foo {
  public $foo;
  public $bar;
  public function __construct($f, $b) {
    echo "Constructing a Foo\n";
    $this->foo = $f;
    $this->bar = $b;
  }
  public function __destruct() {
    echo "Destructing a Foo\n";
  }
  public function __sleep() {
    echo "I'm going to sleep\n";
    return array('foo');
  }
  public function __wakeup() {
    echo "I'm waking up\n";
  }
}

function main() {
  $foo1 = new Foo(1, 2);
  var_dump($foo1);
  apc_store('x', $foo1);
  unset($foo1);
  $foo2 = apc_fetch('x');
  var_dump($foo2);
}

main();
