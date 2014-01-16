<?php

class Foo {
  public function __get($x) {
    var_dump("getter: " . $x);
    if ($x == 'foo') return 42;
    $this->foo++;
    $this->asd++;
  }
}

function main() {
  $x = new Foo;
  $x->asd++;
  var_dump($x);
}

main();

