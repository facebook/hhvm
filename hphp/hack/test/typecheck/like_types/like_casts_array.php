<?hh

function f(mixed $m): void {
  $m as ~varray<int>;
  $m as ~darray<int, arraykey>;
  $m as ~varray_or_darray<arraykey>;
}
