<?php

$bar = new Bar;
$foo = new Foo;

class Bar {
  protected $lol;

  public function __get($x) {
    global $foo;
    echo "Bar heh\n";
    $foo->asd++;
  }
}

class Foo {
  public function __get($x) {
    global $bar;
    echo "Foo heh\n";
    $bar->lol++; // Fatal error
  }
}

$foo->blah++;
var_dump($foo);
var_dump($bar);

