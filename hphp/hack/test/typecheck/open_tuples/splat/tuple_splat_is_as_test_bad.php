<?hh
<<file: __EnableUnstableFeatures('open_tuples', 'type_splat')>>

function test2<<<__Enforceable>> reify T as (mixed...)>(mixed $m): void {
  $m as (int, ...T);
}
