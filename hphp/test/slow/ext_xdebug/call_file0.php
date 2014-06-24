<?php

function foo() {
  var_dump(xdebug_call_file());
}

class Bar {
  function baz() {
    var_dump(xdebug_call_file());
  }
}

var_dump(xdebug_call_file());
foo();
(new Bar())->baz();
