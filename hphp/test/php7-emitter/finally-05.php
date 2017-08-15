<?php

function throw_maybe($x) {
  if ($x) throw $x;
  return 42;
}

function foo($x) {
  try {
    return throw_maybe($x);
  }
  finally {
    echo "hello!\n";
  }
}

function bar($x) {
  try {
    $y = foo($x);
  }
  finally {
    echo "goodbye!\n";
  }
  return $y;
}

var_dump(bar(null));
var_dump(bar(new Exception()));
