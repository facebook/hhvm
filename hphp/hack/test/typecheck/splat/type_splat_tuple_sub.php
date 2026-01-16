<?hh


function test1<T as (mixed...)>((function((...T)): void) $f, T $x): void {
  ($f)($x);
}
function test2<T as (mixed...)>((function(T): void) $f, (...T) $x): void {
  ($f)($x);
}
