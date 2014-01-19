<?php

function foo($p) {
  if ($p) {
    $a = array();
  }
  var_dump((string)$a);
}
foo(false);
