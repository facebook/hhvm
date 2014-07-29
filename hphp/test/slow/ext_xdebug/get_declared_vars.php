<?php

$global1 = 0;
$global2 = "1";

function func1() {
  $func1var = 0;

  function func1inner1() {
    $func2var = true;
    var_dump(xdebug_get_declared_vars());
  }

  $func1inner2 = function ($arg1) use ($func1var) {
    global $global1;
    var_dump(xdebug_get_declared_vars());
  };

  var_dump(xdebug_get_declared_vars());
  func1inner1();
  $func1inner2(1);
}

var_dump(xdebug_get_declared_vars());
func1();
eval("var_dump(xdebug_get_declared_vars());");
require("get_declared_vars_helper.php");
