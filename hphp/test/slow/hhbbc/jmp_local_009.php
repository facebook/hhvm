<?php

class A {}

function foo() {
  if ($x instanceof A) {
    $y = "asd";
  } else {
    $y = "asd2";
  }

  var_dump($y);
}


<<__EntryPoint>>
function main_jmp_local_009() {
foo();
}
