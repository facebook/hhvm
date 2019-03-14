<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function not_in_class(): bool {
  $x = static::class;
  return false;
}
