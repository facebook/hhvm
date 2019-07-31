<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test(int $_): string {
  OOPS:$foobar = test(42);
  $_ = test($foobar);
  return '';
}
