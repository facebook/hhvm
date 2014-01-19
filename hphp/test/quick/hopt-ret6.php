<?php

class C {
  function __destruct() {
    echo "in __destruct\n";
  }
  function simpleRet() {
    return 1;
  }
}

function foo() {
  $x = new C;
  $y = $x->simpleRet();
  var_dump($x);
  var_dump($y);
}

foo();

echo "End\n";
