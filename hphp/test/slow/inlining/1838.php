<?php

/* Compile only: verify no c++ compilation errors */function f($x) {
  return function () use ($x) {
    return $x;
  }
;
}
function g($x) {
  $c = f($x);
  return $c();
}
