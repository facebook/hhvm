<?hh //strict

function take_vec_like(varray<int> $a_): void {}

function test(): void {
  take_vec_like(array_map($x ==> $x, varray[1, 2, 3]));
}
