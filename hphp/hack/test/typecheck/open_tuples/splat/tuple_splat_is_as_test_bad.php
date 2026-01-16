<?hh


function test2<<<__Enforceable>> reify T as (mixed...)>(mixed $m): void {
  $m as (int, ...T);
}
