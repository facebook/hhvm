<?php

$stack = array();

abstract final class PushStackStatics {
  public static $index = 0;
}
function push_stack(){
  global $stack;
  $val = PushStackStatics::$index++;
  array_push(&$stack, $val);
}
function pop_stack(){
  global $stack;
  if ($stack) {
    array_pop(&$stack);
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
