<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

interface J { }
interface I {
  public function foo():this;
}

function gen1<T as I>(T $x):T {
  return $x->foo();
}

function get2<T as I as J>(T $x):T {
  $r = $x->foo();
  return $r;
}

function get3<T>(T $x):T where T as (I&J) {
  $r = $x->foo();
  return $r;
}

function get4<T, T2 as I>(T $x):T where T as (T2&J) {
  $r = $x->foo();
  return $r;
}

function get5<T>(T $y):T {
  $y as I;
  $res = $y->foo();
  return $res;
}
