////file1.php
<?hh

<<__SupportDynamicType>>
class C { }

newtype N<T> as C = C;

function makeN<T>(T $_):N<T> { return new C(); }

newtype NC<+T as dynamic> as C = C;

function makeNC<T as dynamic>(T $_):NC<T> { return new C(); }

////file2.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('upcast_expression')>>

function get():~int {
  return 3;
}

function return_n_direct(N<~int> $n):~N<int> {
  return $n;
}

function return_n():~N<int> {
  // Currently, can't just write return makeN(get());
  //
  // Return constraint:
  //   N<#1> <: ~N<int>
  // iff #1=int \/ #1=~int (push likes)
  //
  // Parameter constraint:
  //   ~int <: #1
  //
  // Problem is that we solve #1=int, and then we're stuck with ~int <: int
  $x = makeN(get());
  return $x;
}

function return_nc_direct(NC<~int> $n):~NC<int> {
  return $n;
}

function return_nc():~NC<int> {
  $x = makeNC(get());
  return $x;
}
