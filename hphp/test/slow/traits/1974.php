<?php

trait A {
  function p() {
    echo "A\n";
  }
}
trait B {
  function p() {
    echo "B\n";
  }
}
trait C {
  function p() {
    echo "C\n";
  }
}
class ABC {
  use A, B, C {
    A::p insteadof B, C;
  }
}
$abc = new ABC();
$abc->p();
?>
