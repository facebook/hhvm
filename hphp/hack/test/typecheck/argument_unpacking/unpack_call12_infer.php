<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function myconcat<Tv>(Traversable<Tv> ...$vec): vec<Tv> {
  $v = vec[];
  foreach ($vec as $x) {
    foreach ($x as $y) {
      $v[] = $y;
    }
  }
  return $v;
}

function test_concat_type(): void {
  $a = vec[vec['foo'], vec['bar']];
  $b = myconcat(...$a);
  expect_vec_of_string($b); // ok
  expect_vec_of_int($b); // error
}

function expect_vec_of_string(vec<string> $v): void {}
function expect_vec_of_int(vec<int> $v): void {}
