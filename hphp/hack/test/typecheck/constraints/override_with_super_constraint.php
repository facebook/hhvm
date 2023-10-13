<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class F {}
interface MyIterable<+Tv> {
  public function concat<Tu super Tv>(): MyIterable<Tu>;
}

class TR<+Trv> implements MyIterable<Trv> {
  // Has to use subtyping to prove this is ok to override. And covariance.
  // And super constraints!
  //  Inherited signature should be
  // <Tu super Trv>(): MyIterable<Tu>
  // So now we need to check MyIterable<Trv> <: MyIterable<Tu>
  // under constraint Tu super Trv i.e. Trv <: Tu
  // So this should hold!
  public function concat<Tu super Trv>(): MyIterable<Trv> {
    throw new Exception();
  }
}
