<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function test(bool $b): dynamic {
  if ($b) {
    return 42;
  } else {
    return;
  }
}
