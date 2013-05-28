<?php

class a extends Exception {
  function __destruct() {
    var_dump('__destruct');
  }
}
;
function foo() {
  $ex = null;
  try {
    throw new A;
  }
 catch (Exception $ex) {
    var_dump(1);
  }
  var_dump(2);
}
foo();
