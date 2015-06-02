<?php

function foo($a) {
  echo __FUNCTION__ . "\n";
}

$x = [1];
foo(&$x[0]);
