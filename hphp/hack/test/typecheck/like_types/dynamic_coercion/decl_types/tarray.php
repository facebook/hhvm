<?hh // partial
// Copyright 2004-present Facebook. All Rights Reserved.

function dyn(): dynamic { return 4; }

function akany(): array {
  return dyn(); // ok
}

function akvec(): array<int> {
  return dyn(); // error
}

function akmap(): array<int, string> {
  return dyn(); // error
}
