<?php

function increment(&$x) {
  $x++;
}
function foo(){
  $foo = array();
  increment(&$foo[0]);
  echo "done\n";
}

<<__EntryPoint>>
function main_base_call_without_warn() {
foo();
}
