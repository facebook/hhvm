<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function test1(vec<vec<~int>> $v):~vec<vec<int>> {
  return $v;
}
function test2(vec<~vec<int>> $v):~vec<vec<int>> {
  return $v;
}
function test3(vec<~vec<~int>> $v):~vec<vec<int>> {
  return $v;
}
function test4(~vec<~vec<~int>> $v):~vec<vec<int>> {
  return $v;
}
function test5(?vec<~int> $v):~?vec<int> {
  return $v;
}
function test6(?vec<~int> $v):?~vec<int> {
  return $v;
}

<<__SupportDynamicType>>
class B<+T> { }
<<__SupportDynamicType>>
class C<+T> extends B<T> { }
<<__SupportDynamicType>>
class D<+T> extends B<T> { }

function chain(~C<int> $c):~B<int> {
  return $c;
}
function test8(C<~int> $c):~B<int> {
  return $c;
}
function test9((C<~int> | D<~int>) $u):~B<int> {
  return $u;
}
function test10((C<~int> | D<~string>) $u):~(C<int> | D<string>) {
  return $u;
}
