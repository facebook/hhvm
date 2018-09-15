<?php

class Foo {
  // PHP's parser doesn't allow trailing commas in function definitions
  public static function fooFunctionWithLongName(
    $barArgumentWithLongName,
    $bazArgumentWithLongName
  ) {
    // PHP's parser doesn't allow trailing commas in function invocations
    return Foo::fooFunctionWithLongName(
      $barArgumentWithLongName,
      $bazArgumentWithLongName
    );
  }
}
