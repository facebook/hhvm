<?php

class C {
}

function foo() {
  $x = new C;
  if (!$x) {
    echo "Error\n";
  }
}

foo();

echo "End\n";
