<?php

abstract final class PushStackStatics {
  public static $index = 0;
  public static $stack = array();
}

function push_stack(){

  $val = PushStackStatics::$index++;
  array_push(&PushStackStatics::$stack, $val);
}
function pop_stack(){

  if (PushStackStatics::$stack) {
    array_pop(&PushStackStatics::$stack);
  }
}
push_stack();
pop_stack();
pop_stack();
pop_stack();
push_stack();
pop_stack();
push_stack();
$info = array(count(PushStackStatics::$stack), PushStackStatics::$stack[count(PushStackStatics::$stack)-1], PushStackStatics::$stack);
var_dump($info);
