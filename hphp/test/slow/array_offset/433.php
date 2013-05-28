<?php

function foo($p) {
  return $p;
}
$a = array(1, 2, 3, 4);
$a[123] = 5;
$a["0000"] = 6;
var_dump(foo(__LINE__));
var_dump(foo(array()));
var_dump(foo(array(1, 2, 3)));
var_dump(foo($a[123]));
var_dump(foo($a[0000]));
var_dump(foo("$a[123]"));
var_dump(foo("$a[0000]"));
