<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function func() :mixed{}

<<__DynamicallyCallable>>
function dyn_func() :mixed{}

class A {
  public static function clsMeth() :mixed{}
}

function expect_exception($arg) :mixed{
  try {
    var_dump(HH\fun_get_function($arg));
  } catch (Exception $e) {
    echo "Caught: ".$e->getMessage()."\n";
  }
}

<<__EntryPoint>>
function main() :mixed{
  expect_exception('func');
  expect_exception(A::clsMeth<>);
  expect_exception(null);
}
