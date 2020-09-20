<?hh // partial
// Copyright 2004-present Facebook. All Rights Reserved.

function foo(): void {
  throw yield 3;
}
