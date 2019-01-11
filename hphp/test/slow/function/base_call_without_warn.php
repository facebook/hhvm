<?php

function increment(&$x) {
  $x++;
}
function name(){
  return 'foo';
}
function foo(){
  increment(&${name()}[0]);
  echo "done\n";
}

<<__EntryPoint>>
function main_base_call_without_warn() {
foo();
}
