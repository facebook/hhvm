<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

final class A { // A is a valid newable constraint
  public function __construct() {}
}

function f<
  <<__Newable>> reify Tfnew as A, // Tfnew must be concrete subtype of A
  reify Tfnotnew as A,  // Tfnotnew can be abstract subtype of A
>(): void {
  g<Tfnew>();
  g<Tfnotnew>();
}

function g<<<__Newable>> reify Tg as A>(): void {}
