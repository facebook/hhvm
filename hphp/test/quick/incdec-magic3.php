<?php

class Dtor { public function __destruct() { echo "Dtor\n"; } }

class Foo {
  public function __get($x) {
    var_dump("getter: " . $x);
    if ($x == 'foo') return 42;
    $this->foo++;
    $this->asd = new Dtor;
  }
}

function main() {
  $x = new Foo;
  $x->asd++;
  var_dump($x);
}

main();
echo "Done\n";
