<?php

class Dtor {
  public function __destruct() { echo "dtor\n"; }
}

class Foo { public $bug; };

function main() {
  $x = new Foo;
  $x->bug = new Dtor;
  $x->bug += 12;
  var_dump($x);
}

main();
echo "done\n";
