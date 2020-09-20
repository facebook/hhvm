<?hh // strict

interface ABD {}
interface BCD {}
interface AB {}
interface AD {}
interface AE {}
interface BC {}
interface BD {}
interface CD {}
interface CE {}

class A implements ABD, AB, AD, AE {}
class B implements ABD, BCD, AB, BC, BD {}
class C implements BCD, BC, CD, CE {}
class D implements ABD, BCD, AD, BD, CD {}
class E implements AE, CE {}

function expect_A(A $x): void {}
function expect_C(C $x): void {}
function expect_E(E $x): void {}
function expect_Eopt(?E $x): void {}
function expect_ABD(ABD $x): void {}
function expect_AB(AB $x): void {}
function expect_AD(AD $x): void {}
function expect_BD(BD $x): void {}
function expect_CE(CE $x): void {}
function expect_ABD_AB_AD_BD_make_Eopt(ABD $x, AB $y, AD $z, BD $t): ?E {
  return new E();
}
function some_cond_of_ABD_AB_AD_BD(ABD $x, AB $y, AD $z, BD $t): bool {
  return true;
}

function test1(bool $b): void {
  $x = new A();
  // error on all but first arg
  for ($i = 0; some_cond_of_ABD_AB_AD_BD($x, $x, $x, $x) && $i < 10; $i++) {
    // $x is A | B | D
    expect_ABD($x);
    expect_AB($x); // error
    expect_AD($x); // error
    expect_BD($x); // error
    $x = new B();
    if ($b) {
      continue;
    }
    $x = new C();
    if ($b) {
      break;
    }
    $x = new D();
  }
  // $x is A | B | C | D
  hh_show($x);
}

function test2(bool $b): void {
  $x = new A();
  for (
    $i = 0;
    // $x is A | B | D (error on all but first arg)
    ($x = expect_ABD_AB_AD_BD_make_Eopt($x, $x, $x, $x)) === null;
    $i++
  ) {
    // $x is ?E
    expect_Eopt($x);
    expect_E($x); // error
    $x = new B();
    if ($b) {
      continue;
    }
    $x = new C();
    if ($b) {
      break;
    }
    $x = new D();
  }
  // $x is C | E
  expect_CE($x);
  expect_C($x); // error
  expect_E($x); // error
}
