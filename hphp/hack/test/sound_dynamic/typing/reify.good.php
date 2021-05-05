<?hh

<<__SupportDynamicType>>
class C<<<__NoRequireDynamic>> reify T> {}

function expect_dynamic(dynamic $d) : void {}

function expect_Cint(C<int> $c) : void {}

function expect_Cdyn(C<dynamic> $c) : void {}

function test(dynamic $d, C<int> $c) : void {
  expect_dynamic($c);
  expect_Cint($d);
  expect_Cdyn($d);
}
