<?php

class C {
  function __destruct() {
    echo "in __destruct\n";
  }
  function simpleRet($x) {
    return 1;
  }
}

function foo() {
  $x = new C;
  $y = $x->simpleRet($x);
  var_dump($x);
  var_dump($y);
}

foo();

echo "End\n";
