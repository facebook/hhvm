<?hh

<<__SupportDynamicType>>
function f(): void {}

function h(): void {
  $x = f<>;
  $x();
}
