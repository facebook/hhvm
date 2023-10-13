<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test1<T as nothing>(T $x): nothing {
  return $x;
}

function test2<T as nothing>(T $x): int {
  return $x;
}

function test3<T as nothing>(?T $x): null {
  return $x;
}
