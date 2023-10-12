<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function foo(mixed $m): void {
  if ($m is nothing) {
    echo "pointless!\n";
  }
}
