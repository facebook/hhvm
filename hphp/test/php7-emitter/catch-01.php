<?php

function foo($exn) {
  try {
    try {
      if ($exn) {
        throw $exn;
      }
    } catch(LogicException $e) {
      return 0;
    }
  } catch (RangeException | OverflowException $e) {
    return 1;
  } catch (Exception $e) {
    return 2;
  }
  return 3;
}

var_dump(foo(null));
var_dump(foo(new Exception()));
var_dump(foo(new RangeException()));
var_dump(foo(new OverflowException()));
var_dump(foo(new LogicException()));
