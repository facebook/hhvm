<?php

class C {
  function __destruct() {
    echo "__destruct!\n";
  }
}

function foo() {
  $x = new C;
  if ($x) {
    echo "not null\n";
  } else {
    echo "null\n";
  }
  $x = 1;
  if ($x) return 1;
}

foo();
