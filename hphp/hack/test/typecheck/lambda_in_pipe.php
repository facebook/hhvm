<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function vec_filter(vec<int> $v, (function (int): bool) $_f): vec<int> {
  return $v;
}

function test(vec<int> $v): vec<int> {
  return $v |> vec_filter($$, function ($x) { return true; });
}
