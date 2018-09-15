<?php
function foo() {
  $x = array();
  while (true) {
    $x[] = str_repeat("asd", 100);
  }
}

function go() {
  echo "hi\n";
  foo();
}


<<__EntryPoint>>
function main_oom_psp() {
ini_set('memory_limit', '18M');

register_postsend_function('go');

foo();
}
