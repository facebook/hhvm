<?php

$stack = array();
function push_stack(){
  global $stack;
  static $index = 0;
  $val = $index++;
  array_push($stack, $val);
}
function pop_stack(){
  global $stack;
  if ($stack) {
    array_pop($stack);
  }
}
push_stack();
pop_stack();
pop_stack();
pop_stack();
push_stack();
pop_stack();
push_stack();
$info = array(count($stack), $stack[count($stack)-1], $stack);
var_dump($info);
