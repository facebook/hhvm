<?php

class C {
  function __destruct() {
    echo "in __destruct\n";
  }
}

function foo() {
  $x = new C;
}

foo();

echo "End\n";
