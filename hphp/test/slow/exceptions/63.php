<?php

ini_set('memory_limit','4M');
function test() {
  for ($i = 0;
 $i < 4000;
 $i++) {
    try {
      call_user_func('bar');
    }
 catch (Exception $e) {
    }
  }
  var_dump('ok');
}
function bar() {
  throw new Exception;
}
test();
