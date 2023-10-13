<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class B { }
class D extends B { }

class Cov<+T> { }

function expectCovB(Cov<B> $cb): void { }
function expectCovD(Cov<D> $cd): void { }

function testit():void {
  $x = new Cov();
  expectCovB($x);
  expectCovD($x);
  $y = new Cov();
  expectCovD($y);
  expectCovB($y);
  // hh_show_env(); should show D as upper bound for both type variables
}
