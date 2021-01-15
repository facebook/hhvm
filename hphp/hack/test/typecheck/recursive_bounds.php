<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class Cov<+Tc> { }
function foo<T2 as T,T as Cov<T2>>(T $x):Cov<T> {
  return $x;
}

class Contra<-TN> { }
function bar<T as Contra<Contra<T>>>(T $x):Contra<T> {
  return $x;
}
