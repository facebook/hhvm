<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function count_to_ten(): Generator<int, int, void> {
  yield 1;
  yield 2;
  yield from varray[3, 4];
  yield from seven_eight();
  yield 9;
  yield 10;
}

function seven_eight(): Generator<int, int, void> {
  yield 7;
  yield from eight();
}

function eight(): Generator<int, int, void> {
  yield 8;
}

function testit(): void {
  foreach (count_to_ten() as $num) {
    echo "$num ";
  }
}
