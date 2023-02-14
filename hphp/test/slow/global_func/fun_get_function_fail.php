<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function func() {}

<<__DynamicallyCallable>>
function dyn_func() {}

class A {
  public static function clsMeth() {}
}

function expect_exception($arg) {
  try {
    var_dump(HH\fun_get_function($arg));
  } catch (Exception $e) {
    echo "Caught: ".$e->getMessage()."\n";
  }
}

<<__EntryPoint>>
function main() {
  expect_exception('func');
  expect_exception(A::clsMeth<>);
  expect_exception(null);
}
