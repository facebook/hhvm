<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function good(~string $s): int {
  return $s as dynamic;
}

function bad(~string $s): int {
  return $s;
}
