<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function throws<T>(TypeStructure<T> $ts, mixed $value): T {
  return /* UNSAFE_EXPR */ $value;
}

function testit(mixed $value, TypeStructure<Map<string, int>> $ts): void {
  $f = (int $arg) ==> {
    if ($value === null) {
      return null;
    } else {
      $y = throws($ts, $value);
      // This causes error but return $y does not
      return (throws($ts, $value));
    }
  };
}
