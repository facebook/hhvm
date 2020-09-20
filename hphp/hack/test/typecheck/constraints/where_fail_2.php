<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

interface I { }
class C<T as I> { }

class D<Td as I> {
  public function bounded1<T as C<T>>(T $x):void { }

  // This should succeed. Despite appearances, order of `as` doesn't matter
  public function bounded2<T as C<T> as I>(T $x):void { }

  // This should succeed
  public function bounded3<T as C<Td>>(T $x):void { }

  // This should fail, just like bounded1
  public function where1<T>(T $x):void where T as C<T>  { }

  // This should succeed, just like bounded2
  public function where2<T as I>(T $x):void where T as C<T> { }

  // This should succeed, just like bounded3
  public function where3<T>(T $x):void where T as D<Td> { }
}
