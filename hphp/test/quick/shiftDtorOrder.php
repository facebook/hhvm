<?php

class A {
    public function __destruct() { echo "~A\n"; }
}

class B {
    public function __destruct() { echo "~B\n"; }
}

function main() {
    $k = (new A) << (new B);
      var_dump($k);
}
main();

