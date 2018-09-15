<?php

class C {
 }
function foo($p) {
  if ($p) {
    $obj = new C;
  }
 else {
    $a = array(1);
  }
  var_dump($obj == $a);
}

<<__EntryPoint>>
function main_1053() {
foo(false);
}
