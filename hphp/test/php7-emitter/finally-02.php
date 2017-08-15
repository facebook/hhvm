<?php

function foo($x) {
  try {
    if ($x) {
      throw $x;
    }
    return 1;
  }
  catch (LogicException $e) {
    return 2;
  }
  catch (RuntimeException $e) {
    return 3;
  }
  catch (Exception $e) {
    return 4;
  }
  finally {
    return 10;
  }
  return "foo";
}

var_dump(foo(null));
var_dump(foo(new LogicException()));
var_dump(foo(new RuntimeException()));
var_dump(foo(new Exception()));
