<?hh
<<file:__EnableUnstableFeatures('case_types')>>

case type CT_Bounded as arraykey, XHPChild = int;

case type CT_No_Bounds = int;

function expect_int(int $x): void {}
function expect_xhpchild(XHPChild $x): void {}
function expect_ak(arraykey $x): void {}
function expect_mixed(mixed $x): void {}
function expect_bounded(CT_Bounded $x): void {}
function expect_unbounded(CT_No_Bounds $x): void {}

function bounded(CT_Bounded $bounded, CT_No_Bounds $unbounded): void {
  expect_int($bounded); // Error
  expect_xhpchild($bounded); // Ok
  expect_ak($bounded); // Ok
  expect_mixed($bounded); // Ok
  expect_bounded($bounded); // Ok
  expect_unbounded($bounded); // Error

  expect_int($unbounded); // Error
  expect_xhpchild($unbounded); // Error
  expect_ak($unbounded); // Error
  expect_mixed($unbounded); // Ok
  expect_bounded($unbounded); // Error
  expect_unbounded($unbounded); // Ok
}
