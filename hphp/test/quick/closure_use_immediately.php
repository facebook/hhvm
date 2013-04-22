<?php

class A {}

function a() {
  $b = new A;
  $c = function() use ($b) {
    return $b;
  };
  return $c();
}

var_dump(a());
