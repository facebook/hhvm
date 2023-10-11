<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

/* Sanity check for parametrized reified generics, makes sure the global
 * tpenv correctly reports them as reified */
class A {}
class B extends A {}
class C extends B {}

function f<
  reify Tassuper as A super C,
  reify Teq
>(): void where Teq = B {}

function bad<Tbad1 as B super B, Tbad2>(): void where Tbad2 = B {
  f<Tbad1, Tbad2>();
}

function good<reify Tgood1 as B super B, reify Tgood2>(): void where Tgood2 = B {
  f<Tgood1, Tgood2>();
}
