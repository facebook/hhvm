<?hh //strict

function take_vec_like(array<int> $a_): void {}

function test(): void {
  take_vec_like(array_map($x ==> $x, array(1, 2, 3)));
}
