<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function foo(string $s) : int {
  switch ($s) {
    default:
      return 0;
      break;
    case '1':
      return 1;
    case '2':
      return 2;
  }
}
