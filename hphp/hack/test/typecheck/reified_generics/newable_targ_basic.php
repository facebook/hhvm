<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

enum E: int {}
trait Tr {}
interface I {}
abstract class A {}
class C {}
type Ty = C;

// Note the lack of reify, only testing __Newable
function f<<<__Newable>> T>(): void {}

function non_happly(): void {
  f<Ty>();
  f<dynamic>();
}

function not_concrete(): void {
  f<E>();
  f<I>();
  f<A>();
  f<Tr>();
}

function concrete(): void {
  // No error here, but f still has an error because T is not constrained, so
  // it won't know the appropriate constructor for C
  f<C>();
}
