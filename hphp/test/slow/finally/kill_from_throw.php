<?php

function foo() {
  try {
    try {
      return 0;
    } finally {
      throw new Exception('lol');
    }
  } catch (Exception $e) {
    var_dump("catch");
  }
  var_dump("after");
}
var_dump(foo());
