<?php

function foo($x, $y) {
  try {
    if ($x) return $x;
  } finally {
    try{
      if ($y) return $y;
    } finally{
      echo "inner\n";
    }
    echo "outer\n";
  }
  return 42;
}

var_dump(foo(false, false));
var_dump(foo("x set", false));
var_dump(foo("x set", "x and y set"));
var_dump(foo(false, "just y set"));
