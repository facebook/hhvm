<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function make_error() :mixed{
  $a = dict['x' => 2];
  $a->foo;
}

function error_handler1() :mixed{
  echo "===================== error_handler1 =========================\n";
  var_dump(HH\deferred_errors());
  make_error();
  var_dump(HH\deferred_errors());
  var_dump(HH\deferred_errors());
  echo "==============================================================\n";
}

function error_handler2() :mixed{
  echo "===================== error_handler2 =========================\n";
  var_dump(count(HH\deferred_errors()));
  for ($i = 0; $i < 100; $i++) {
    make_error();
  }
  $errors = HH\deferred_errors();
  var_dump(count($errors));
  var_dump(count(HH\deferred_errors()));
  var_dump($errors[49]['overflow']);
  echo "==============================================================\n";
}

function error_handler3() :mixed{
  echo "===================== error_handler3 =========================\n";
  make_error();
  make_error();
  make_error();
  echo "==============================================================\n";
}
<<__EntryPoint>> function main(): void {
error_reporting(0);

var_dump(HH\deferred_errors());

set_error_handler(error_handler1<>);
make_error();

set_error_handler(error_handler2<>);
make_error();

set_error_handler(error_handler3<>);
make_error();
var_dump(HH\deferred_errors());
}
