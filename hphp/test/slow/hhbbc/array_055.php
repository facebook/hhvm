<?php

function foo() {
  $x = array();
  $x[new stdclass] = 2;
  var_dump($x);
}
foo();
