<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

interface I {}
class A {}

class Ctv<-T as I as A> {}

function test(): void {
  $x = new Ctv();
  hh_show($x);
}
