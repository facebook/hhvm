<?hh

<<file: __EnableUnstableFeatures('open_tuples', 'type_splat')>>

function expect_dyn(dynamic $_):void { }

<<__NoAutoDynamic>>
class C { }

function test<<<__NoAutoBound>> T as (C...)>((int, ...T) $tup):void {
  expect_dyn($tup);
}
