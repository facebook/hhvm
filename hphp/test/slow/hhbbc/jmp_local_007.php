<?php

function foo($x) {
  if ($x) {
    $y = "asd";
  } else {
    $y = "asd2";
  }

  var_dump($y);
}

foo();
