<?hh // partial

function foo(mixed $x) {
  if ($x is varray<_>) {
    hh_show($x);
  } else if ($x is darray<_, _>) {
    hh_show($x);
  } else if ($x is varray<_>) {
    hh_show($x);
  } else if ($x is darray<_, _>) {
    hh_show($x);
  } else if ($x is varray_or_darray<_>) {
    hh_show($x);
  }
}
