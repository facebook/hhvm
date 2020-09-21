<?hh

<<file: __EnableUnstableFeatures('coeffects_provisional')>>
function f()@{ \HH\Contexts\pure }: void {
  f();
  g();
}
function g()@{ \HH\Contexts\non_rx }: void {
  f();
  g();
}
