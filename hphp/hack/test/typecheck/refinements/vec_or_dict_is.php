<?hh

function test_is_vec(vec_or_dict<string, bool> $x): void {
  if ($x is vec<_>) {
    hh_show($x);
  } else {
    hh_show($x);
  }
}

function test_is_dict(vec_or_dict<string, bool> $x): void {
  if ($x is dict<_, _>) {
    hh_show($x);
  } else {
    hh_show($x);
  }
}

function is_vec_vec_or_dict_with_string_key(
  vec<bool> $x,
): vec_or_dict<string, bool> {
  return $x;
}
