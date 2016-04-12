<?php

function foo() {
  for ($i = 0; $i < 100; $i++) {
    echo "Count: $i\n";
  }
}

class X {
  function __destruct() {
    var_dump(__METHOD__);
    sleep(2);
    foo();
  }
}

class Y {
  function __destruct() {
    var_dump(__METHOD__);
  }
}

set_error_handler(function() {
    var_dump('Handler', func_get_args());
  }, -1);

function test() {
  $x = new X;
  $y = new Y;
  set_time_limit(1);
}

test();
