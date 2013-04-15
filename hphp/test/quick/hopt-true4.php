<?php

function foo($x) {
  if (!$x) { return true; }
  else { return false; }
}

var_dump(foo(true));
var_dump(foo(1));
