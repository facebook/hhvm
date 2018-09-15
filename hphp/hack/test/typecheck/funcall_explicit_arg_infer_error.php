<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function id<T>(T $x): T { return $x; }

function infer_and_error(int $x): string {
  return id<_>($x);
}
