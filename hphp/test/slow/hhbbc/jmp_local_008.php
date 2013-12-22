<?php

function foo($x) {
  if (is_string($x)) {
    $y = "asd";
  } else {
    $y = "asd2";
  }

  var_dump($y);
}

foo();
