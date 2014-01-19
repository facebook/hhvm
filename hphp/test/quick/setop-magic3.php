<?php

class Dtor { public function __destruct() { echo "Dtor\n"; } }

class Foo {
  public function __get($x) {
    var_dump("getter: " . $x);
    if ($x == 'foo') return 12;
    $this->foo += 12;
    $this->asd = new Dtor;
  }
}

function main() {
  $x = new Foo;
  $x->asd += 12;
  var_dump($x);
}

main();
echo "Done\n";

