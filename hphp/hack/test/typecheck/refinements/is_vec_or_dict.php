<?hh

case type VecOrInt = vec<int> | int;

function test_vec_or_int(VecOrInt $x): int {
  if ($x is vec_or_dict<_>) {
    return 0;
  } else {
    return $x;
  }
}

function test_vec_or_dict_key(Container<mixed> $x): void {
  if ($x is vec_or_dict<_>) {
    foreach ($x as $k => $v) {
      $x[$k] = null;
    }
  }
}

function test_vec_or_dict_key2(Container<mixed> $x): void {
  if ($x is vec_or_dict<_, _>) {
    foreach ($x as $k => $v) {
      $x[$k] = null;
    }
  }
}

case type KCOrInt = KeyedContainer<int, bool> | int;

function test_keyed_container(KCOrInt $x): void {
  if ($x is vec_or_dict<_>) {
    hh_show($x);
    hh_expect<vec_or_dict<int, bool>>($x);
  } else {
    hh_show($x);
  }
}
