<?php

class C {
  public function __destruct() {
    echo "C::__destruct\n";
    global $obj;
    var_dump($obj);
    $obj = 1;
    echo "Leaving C::__destruct\n";
  }
}

class D {
  public function __destruct() {
     echo "D::__destruct\n";
  }
}

function main() {
  global $obj;
  $obj = new C;
  $obj = new D;
  echo "****\n";
  var_dump($obj);
}

main();
