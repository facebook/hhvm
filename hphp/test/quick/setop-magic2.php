<?php

$bar = new Bar;
$foo = new Foo;

class Bar {
  protected $lol;

  public function __get($x) {
    global $foo;
    echo "Bar heh\n";
    $foo->asd += 1;
  }
}

class Foo {
  public function __get($x) {
    global $bar;
    echo "Foo heh\n";
    $bar->lol += 1; // Fatal error
  }
}

$foo->blah += 1;
var_dump($foo);
var_dump($bar);

