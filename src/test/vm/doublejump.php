<?php

function f() {
  $a = true;
  $b = true;
  $i = 0;

  if($a) {
    if($b) {
      $i = 1;
    } else {
      $i = 2;
    }
  } else {
    if($b) {
      $i = 3;
    } else {
      $i = 4;
    }
  }
}

function main() {
  for($i = 0; $i < 1000; $i++) {
    f();
  }
}
main();

