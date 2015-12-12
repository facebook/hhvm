<?php

assert_options(ASSERT_ACTIVE, true);
assert_options(ASSERT_WARNING, true);

function f() {
  assert('false');
  assert('g()');
}

function g() {
  throw new Exception();
}

f();
