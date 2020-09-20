<?hh // partial
// Copyright 2004-present Facebook. All Rights Reserved.

function akany(~array $a): array {
  return $a; // ok
}

function akvec(~array<int> $a): array<int> {
  return $a; // error
}

function akmap(~array<int, string> $a): array<int, string> {
  return $a; // error
}
