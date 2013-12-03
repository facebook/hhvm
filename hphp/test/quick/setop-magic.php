<?php

class Foo {
  public function __get($x) {
    var_dump("getter: " . $x);
    if ($x == 'foo') return 12;
    $this->foo += 12;
    $this->asd += 12;
  }
}

function main() {
  $x = new Foo;
  $x->asd += 12;
  var_dump($x);
}

main();

