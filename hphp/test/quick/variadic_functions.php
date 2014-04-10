<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function variadic_only(...$v) {
  echo "\n", '* ', __FUNCTION__, "\n";
  var_dump(func_get_args());
  var_dump(isset($v));
}

function variadic_some($x, ...$v) {
  echo "\n", '* ', __FUNCTION__, "\n";
  var_dump(func_get_args());
  var_dump(isset($v));
}

function variadic_hack_only(...) {
  echo "\n", '* ', __FUNCTION__, "\n";
  var_dump(func_get_args());
}

function variadic_hack_some($x, ...) {
  echo "\n", '* ', __FUNCTION__, "\n";
  var_dump($x);
  var_dump(func_get_args());
}

class C {
  public function variadic_only(...$v) {
    echo "\n", '* ', __METHOD__, "\n";
    var_dump(func_get_args());
    var_dump(isset($v));
  }

  public function variadic_some($x, ...$v) {
    echo "\n", '* ', __METHOD__, "\n";
    var_dump(func_get_args());
    var_dump(isset($v));
  }

  public function variadic_hack_only(...) {
    echo "\n", '* ', __METHOD__, "\n";
    var_dump(func_get_args());
  }

  public function variadic_hack_some($x, ...) {
    echo "\n", '* ', __METHOD__, "\n";
    var_dump($x);
    var_dump(func_get_args());
  }

  public static function st_variadic_only(...$v) {
    echo "\n", '* ', __METHOD__, "\n";
    var_dump(func_get_args());
    var_dump(isset($v));
  }

  public static function st_variadic_some($x, ...$v) {
    echo "\n", '* ', __METHOD__, "\n";
    var_dump(func_get_args());
    var_dump(isset($v));
  }

  public static function st_variadic_hack_only(...) {
    echo "\n", '* ', __METHOD__, "\n";
    var_dump(func_get_args());
  }

  public static function st_variadic_hack_some($x, ...) {
    echo "\n", '* ', __METHOD__, "\n";
    var_dump($x);
    var_dump(func_get_args());
  }
}

// TODO: async functions

// TODO: continuations

// TODO: closure bodies

// TODO: constructors

// TODO: constructor with modifier

function main() {
  variadic_only('a', 'b', 'c');
  variadic_some('a', 'b', 'c');
  variadic_hack_only('a', 'b', 'c');
  variadic_hack_some('a', 'b', 'c');
  echo "\n", '========= static methods ==========', "\n";
  C::st_variadic_only('a', 'b', 'c');
  C::st_variadic_some('a', 'b', 'c');
  C::st_variadic_hack_only('a', 'b', 'c');
  C::st_variadic_hack_some('a', 'b', 'c');
  echo "\n", '========= instance methods ==========', "\n";
  $inst = new C();
  $inst->variadic_only('a', 'b', 'c');
  $inst->variadic_some('a', 'b', 'c');
  $inst->variadic_hack_only('a', 'b', 'c');
  $inst->variadic_hack_some('a', 'b', 'c');
}

main();
