<?php

class A {}

function foo($x) {
  if ($x instanceof A) {
    $y = "asd";
  } else {
    $y = "asd2";
  }

  var_dump($y);
}

foo();
