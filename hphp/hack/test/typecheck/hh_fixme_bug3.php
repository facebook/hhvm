<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C<T> { }
class D<+T> { }
function foo<Tk as string>(
  C<Tk> $input,
): D<Tk> {
  throw new Exception("E");
}
function expectDString(D<string> $s):void { }
function testit(C<arraykey> $d):void {
  /* HH_FIXME[4323] */
  $x = foo<_>($d);
  expectDString($x);
}
