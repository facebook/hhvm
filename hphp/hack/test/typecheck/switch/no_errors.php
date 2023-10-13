<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function foo(string $s) : int {
  switch ($s) {
    case '1':
      return 1;
    case 'abc':
    default:
      return 0;
      break;
  }
}
