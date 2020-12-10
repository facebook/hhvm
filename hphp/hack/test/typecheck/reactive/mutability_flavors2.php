<?hh
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class A {}

<<__Rx>>
function f(): void {
  $a = \HH\Rx\mutable(new A());
  // OK
  $b = \HH\Rx\move($a);
  // OK
  $a = \HH\Rx\mutable(new A());
  // OK
  $a = \HH\Rx\move($a);
}
