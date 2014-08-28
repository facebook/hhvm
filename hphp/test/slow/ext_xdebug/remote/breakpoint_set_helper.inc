<?php
class Foo {
  function method() {
    var_dump(__FUNCTION__);
  }

  function method2() {
    var_dump(__FUNCTION__);
  }
}

function bar($x) {
  var_dump(__FUNCTION__);
  return $x;
}

function call() {
  var_dump(__FUNCTION__);
}

function ret($x) {
  return $x * $x;
}

var_dump(__FILE__);
