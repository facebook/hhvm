<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function id<Tk as arraykey, Tv>(dict<Tk, Tv> $d): dict<Tk, Tv> {
  return $d;
}

function  singleton<Tk as arraykey, Tv>(Tk $k, Tv $v): dict<Tk, Tv> {
  return dict[$k => $v];
}

function test(vec<vec<vec<int>>> $v): void {
  $d = id(singleton(0, $v));
  $d[4][3][2][] = 0;
  $d[4][3][2][] = 0;
  $d[4][3][2][] = 0;
  $d[4][3][2][] = 0;
  $d[4][3][2][] = 0;
  $d[4][3][2][] = 0;
}
