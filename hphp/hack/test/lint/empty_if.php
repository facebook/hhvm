<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function empty_if(): void {
  $foo = true;
  if ($foo) {}
}
