<?php

/*
 * Test intercepts where callsites have already been bound to the
 * pre-intercept function.
 */

function foo() {
  var_dump(__METHOD__);
}

function bar() {
  var_dump(__METHOD__);
}

function test() {
  foo();
}

class C {
  function __call($name, $args) {
    var_dump(__METHOD__, $name, $args);
  }
}


function swizzle($name, $obj, $args, $data, &$done) {
  var_dump($name, $obj, $args, $data, $done);
  $done = false;
}

function main() {
  $c = new C();
  for ($i = 0; $i < 3; $i++) {
    test();
    foo();
    $c->snoot();
    if ($i == 1) {
      fb_intercept('foo', 'bar', false);
      fb_intercept('C::__call', 'swizzle');
    }
  }
}

main();

