<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function f<Tk>(Vector<?Tk> $x): Vector<Tk> {
  return Vector {};
}

function test1(Vector<?int> $x): Vector<?int> {
  return f($x);
}

function test2(Vector<?int> $x): Vector<int> {
  return f($x);
}
