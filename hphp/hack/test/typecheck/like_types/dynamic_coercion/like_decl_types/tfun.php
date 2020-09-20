<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function f(~(function (): string) $a): (function (): string) {
  return $a; // error
}
