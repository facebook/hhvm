<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function enforceable(~?int $i): ?int {
  return $i; // corner case: ?(dynamic | int)
}

function enforceable2(?~int $i): ?int {
  return $i; // ok
}

function unenforceable(~?(int, int) $i): ?(int, int) {
  return $i; // error
}
