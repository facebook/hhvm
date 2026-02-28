<?hh

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
function expect_Copt(?C $x): void {}
function expect_Eopt(?E $x): void {}
function expect_E(E $x): void {}
function expect_ABD(ABD $x): void {}
function expect_BCD(BCD $x): void {}
function expect_AB(AB $x): void {}
function expect_AD(AD $x): void {}
function expect_AE(AE $x): void {}
function expect_BC(BC $x): void {}
function expect_BD(BD $x): void {}
function expect_CD(CD $x): void {}
function expect_CEopt(?CE $x): void {}
function expect_CE(CE $x): void {}
function expect_BD_B_D_make_Eopt(BD $x, B $y, D $z): ?E {
  return new E();
}
function some_cond_of_BD_B_D(BD $x, B $y, D $z): bool {
  return true;
}

function test1(bool $b): void {
  $x = new A();
  do {
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
  } while (
    // $x is B | D
    some_cond_of_BD_B_D($x, $x, $x) // errors on 2nd and 3rd args
  );
  // $x is B | C | D
  expect_BCD($x);
  expect_BC($x); // error
  expect_BD($x); // error
  expect_CD($x); // error
}

function test2(bool $b): void {
  $x = new A();
  do {
    // $x is A | E
    expect_AE($x);
    expect_A($x); // error
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
  } while (
    // $x is B | D (errors on 2nd and 3rd args)
    ($x = expect_BD_B_D_make_Eopt($x, $x, $x)) !== null
  );
  // $x is C | ?E
  expect_CEopt($x);
  expect_CE($x); // error
  expect_Copt($x);
  expect_Eopt($x); // error
}
