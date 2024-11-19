<?hh

<<file: __EnableUnstableFeatures('open_tuples', 'type_splat')>>

function expect_dyn(dynamic $_):void { }

<<__SupportDynamicType>>
class C { }

function test<T as (C...)>((int, ...T) $tup):void {
  expect_dyn($tup);
}
