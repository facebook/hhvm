<?php

function foo(&$str) {
  $str[3] = '.';
  $str .= "\n";
}

$a = "abc";
foo($a);
var_dump($a);
