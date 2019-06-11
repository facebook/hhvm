<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function f(dynamic $d): void {
  // the runtime will enforce both the parameter and return types of a lambda
  // so long as they are declared (and enforceable)
  $f = (int $i): int ==> {
    return $d;
  };
  $f($d);
}

function g(dynamic $d): void {
  $f = ((int, string) $i): (int, string) ==> {
    return $d;
  };
  $f($d);
}
