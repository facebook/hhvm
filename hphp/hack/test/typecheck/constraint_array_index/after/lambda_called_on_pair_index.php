<?hh
function f(): void {
  $f = ($v ==> Pair {"a", "b"}[$v]);
  $f(0);
}
