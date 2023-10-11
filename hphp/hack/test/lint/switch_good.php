<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test1(num $x): void {
  switch ($x) {
    case 42: return;
    case 3.14: return;
    default: return;
  }
}

function test2(arraykey $x): void {
  switch ($x) {
    case 42: return;
    case 'foo': return;
    default: return;
  }
}

class C {}

function test3(vec<C> $x): void {
  switch ($x) {
    case vec[]: return;
    case vec[new C()]: return;
    default: return;
  }
}
