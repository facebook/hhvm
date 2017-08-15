<?php

function foo($x) {
  $y = 0;
  try {
    if ($x) {
      throw $x;
    }
    $y = 1;
  }
  catch (LogicException $e) {
    $y = 2;
  }
  catch (RuntimeException $e) {
    $y = 3;
  }
  catch (Exception $e) {
    $y = 4;
  }
  finally {
    $y *= 10;
  }
  return $y;
}

var_dump(foo(null));
var_dump(foo(new LogicException()));
var_dump(foo(new RuntimeException()));
var_dump(foo(new Exception()));
