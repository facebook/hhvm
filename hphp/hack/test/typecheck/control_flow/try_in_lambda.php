<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function might_throw(): void {}

function foo(vec<int> $v): void {
  $y ==> {
    try {
      foreach ($v as $x) {
        might_throw();
      }
    } catch (Exception $e) {
      $x;
    }
  };
}
