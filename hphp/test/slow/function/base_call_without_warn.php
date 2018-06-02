<?php

function increment(&$x) {
  $x++;
}
function name(){
  return 'foo';
}
function foo(){
  increment(${name()}[0]);
  echo "done\n";
}
foo();
