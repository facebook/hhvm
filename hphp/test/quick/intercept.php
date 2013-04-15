<?php

function foo() { var_dump(__METHOD__); }
function bar() {
  var_dump(__METHOD__);
  throw new Exception;
}

function boo() {
  fb_intercept('foo', 'bar', 'bar');
  try {
    foo();
  } catch (Exception $e) {
    var_dump("caught:" . $e->getMessage());
  }
}

boo();
