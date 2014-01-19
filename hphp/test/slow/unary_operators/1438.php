<?php

function foo($x) {
  if ($x) {
    $a = array();
    $s = 'hello';
    $o = (object)null;
  }
  var_dump((array)$a, (array)$s, (array)$o);
  var_dump((string)$a, (string)$s, (string)$o);
  var_dump((object)$a);
var_dump((object)$s);
var_dump((object)$o);
}
foo(false);
