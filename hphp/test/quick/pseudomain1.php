<?php

class C {
  public function __destruct() {
    echo "C::__destruct\n";
    global $obj;
    var_dump($obj);
    echo "==========\n";
    $obj = 1;
  }
}

class D {
  public function __destruct() {
    echo "D::__destruct\n";
  }
}

$obj = new C;
$obj = new D;
echo "****\n";
var_dump($obj);
