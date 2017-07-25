<?php

function &bar() {
  return 2;
}

$x =& bar();
var_dump($x);
