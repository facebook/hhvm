<?hh
<<file: __EnableUnstableFeatures('open_tuples', 'type_splat')>>

function test1<T as (mixed...)>((function((...T)): void) $f, T $x): void {
  ($f)($x);
}
function test2<T as (mixed...)>((function(T): void) $f, (...T) $x): void {
  ($f)($x);
}
