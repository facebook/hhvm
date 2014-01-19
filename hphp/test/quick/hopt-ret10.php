<?php

function foo($x) {
  $y =& $x;
  $y = "NEW";
  if (!$y) { echo "error"; }
  return $y;
}

echo foo("OLD") . "\n";

