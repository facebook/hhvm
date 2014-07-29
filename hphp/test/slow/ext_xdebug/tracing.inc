<?php

var_dump(xdebug_get_tracefile_name());
require('tracing_helper.inc');

class Foo {
  function bar($i) {
    if ($i == 0) return baz();
    return $this->bar($i - 1);
  }
}

function baz() {
  boo(); boo();
  boo();
  return 10;
}

function boo() {}

(new Foo())->bar(3);
boo();
bee();
