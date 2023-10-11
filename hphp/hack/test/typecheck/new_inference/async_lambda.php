<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test(bool $b): void {
  $c = $b ? Vector {} : Set {};
  $c->map(async $_ ==> 42);
}
