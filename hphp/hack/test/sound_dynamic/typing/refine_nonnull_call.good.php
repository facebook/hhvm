<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<__SupportDynamicType>>
class C { }
<<__SupportDynamicType>>
class D { }
<<__SupportDynamicType>>
function getC():~?C { return new C(); }
<<__SupportDynamicType>>
function foo(C $_):D { return new D(); }

<<__SupportDynamicType>>
function bar():void {
  $nc = getC();
  $nc as nonnull;
  // We should allow argument to have type (dynamic & nonnull) | C
  $ld = foo($nc);
  // But result should be ~D because the function
  // essentially has the type (C->D) & (dynamic->dynamic)
  hh_expect_equivalent<~D>($ld);
}
