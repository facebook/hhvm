<?php

class C {
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
