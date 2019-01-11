<?php

abstract class Base {
  public function foo(&$x) {
    $this->implFoo($x);
  }

  public function implFoo(&$x = null, $y = false) {}
}

abstract class Derived extends Base {
}

class D2 extends Derived {
  public function implFoo() {
    echo "hi\n";
  }
}

function main() {
  $x = new D2;
  $y = 'ok';
  $x->foo(&$y);
}


<<__EntryPoint>>
function main_func_family_013() {
main();
}
