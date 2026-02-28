<?hh

<<__SupportDynamicType>>
class C {}

<<__SupportDynamicType>>
function getC(): ~?C {
  return new C();
}

<<__SupportDynamicType>>
function foo(C $_): void {}

<<__SupportDynamicType>>
function bar(): void {
  $nc = getC();
  $nc as nonnull;
  foo($nc);
}
