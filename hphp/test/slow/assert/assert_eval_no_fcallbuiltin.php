<?php

function f() {
  assert('false');
  assert('g()');
}

function g() {
  throw new Exception();
}


<<__EntryPoint>>
function main_assert_eval_no_fcallbuiltin() {
assert_options(ASSERT_ACTIVE, true);
assert_options(ASSERT_WARNING, true);

f();
}
