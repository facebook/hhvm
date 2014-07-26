<?php

require('profiling_testee_helper.inc');

function foo($i) {
  if ($i == 0) return bar();
  else return foo($i - 1);
}

function bar() {
  return 10;
}

class Baz {
  function bar() {
    $this->boo();
    $this->boo(); $this->boo();
  }
  function boo() {
  }
}

var_dump(xdebug_call_file());
(new Baz())->bar();
var_dump(time());
var_dump(foo(2));
echo xdebug_get_profiler_filename();
