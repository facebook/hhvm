<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

function foo<T super vec<int>>(vec<~int> $v):~T {
  return $v;
}
function bar<T super (int,string)>((~int,string) $p):~T {
  return $p;
}
function boo<T super vec<int> as supportdyn<mixed>>(vec<~int> $v):~T {
  return $v;
}
