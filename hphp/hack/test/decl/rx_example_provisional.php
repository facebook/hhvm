<?hh

<<file: __EnableUnstableFeatures('coeffects_provisional')>>
function f()@{ rx }: void {
  f();
  g();
}
function g()@{ defaults }: void {
  f();
  g();
}
