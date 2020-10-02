<?hh

function f(varray_or_darray $partial, varray<int> $checked1, darray<string, int> $checked2) {
  $a = $partial;
  $a = $checked1;
  $a = $checked2;
}
