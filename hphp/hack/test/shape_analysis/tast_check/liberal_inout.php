<?hh

function f(mixed $_, inout mixed $_): void {}

function test(): void {
  $d1 = dict[]; // Not invalidated
  $d2 = dict[]; // Not invalidated
  f($d1, inout $d2);
  $v1 = vec[dict[]]; // Not invalidated
  $v2 = vec[dict[]]; // Invalidated because we don't know what flows back into $d2
  $v1[] = $d1;
  $v2[] = $d2;
}
