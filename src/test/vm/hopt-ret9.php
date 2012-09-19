<?php

class C {
  function __destruct() {
    echo "in __destruct\n";
  }
}

function foo() {
  $x = new C;
  if (!$x) {
    echo "Error\n";
  }
}

foo();

echo "End\n";
