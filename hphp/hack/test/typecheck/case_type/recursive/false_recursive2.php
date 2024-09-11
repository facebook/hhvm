<?hh
<<file: __EnableUnstableFeatures('case_types')>>

case type A<T> = T;

function f(string $x): void {
  hh_expect<A<A<string>>>($x);
}
