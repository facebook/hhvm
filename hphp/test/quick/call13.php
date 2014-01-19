<?php
class C {
}
class D extends C {
  public function __call($fn, $args) {
    echo "D::__call\n";
  }
}
class E extends D {
  public function __call($fn, $args) {
    echo "E::__call\n";
  }
  public function test() {
    $this->foo();
    D::foo();
    E::foo();
  }
}
class F extends D {
  public function __call($fn, $args) {
    echo "F::__call\n";
  }
}

function main() {
  $obj = new E;
  $obj->test();

  $obj->foo();
  E::foo();
}

main();

