<?hh

<<__SupportDynamicType>>
class C<T> {}

<<__SupportDynamicType>>
function f(): void {}

<<__SupportDynamicType>>
function g<T>(C<T> $c): void {
  // SDT pass should not error about $#capability
  f();
}
