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
  return makeN(get());
}

function return_nc_direct(NC<~int> $n):~NC<int> {
  return $n;
}

function return_nc():~NC<int> {
  $x = makeNC(get());
  return $x;
}
