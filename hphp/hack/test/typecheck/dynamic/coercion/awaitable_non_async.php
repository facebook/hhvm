<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function f(Awaitable<int> $i): Awaitable<dynamic> {
  return $i;
}
