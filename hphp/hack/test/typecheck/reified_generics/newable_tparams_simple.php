<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

<<__ConsistentConstruct>>
class X {}
class Y extends X {}

<<__ConsistentConstruct>>
class XX {}
class YY extends XX {}

class Z {}

class C<
  // Bounds are collaped to `as Y as YY`
  <<__Newable>> reify Tc1 as X as Y as XX as YY,
  <<__Newable>> reify Tc2 as Z,
  // consistent constraint must be an upper bound, not lower
  <<__Newable>> reify Tc3 super X,
> {}
