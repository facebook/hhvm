<?hh
<<file: __EnableUnstableFeatures('case_types')>>

case type Lst = ?Cons::T;

class Cons {
  const type T = (int, Lst);
}

function reverse_list(Lst $list): Lst {
  $list_acc = null;
  while ($list is (mixed, mixed)) {
    list($elt, $list) = $list;
    $list_acc = tuple($elt, $list_acc);
  }
  return $list_acc;
}

function rev_vec_to_list(vec<int> $vec): Lst {
  $list = null;
  foreach ($vec as $elt) {
    $list = tuple($elt, $list);
  }
  return $list;
}

function vec_to_list_rec(vec<int> $vec, int $len, int $i): Lst {
  if ($i === $len) {
    return null;
  }
  $elt = $vec[$i];
  $list = vec_to_list_rec($vec, $len, $i + 1);
  return tuple($elt, $list);
}

function vec_to_list(vec<int> $vec): Lst {
  $len = vec_length($vec);
  return vec_to_list_rec($vec, $len, 0);
}

function rec_vec_to_list_tail_rec(
  vec<int> $vec,
  int $len,
  int $i,
  Lst $list_acc,
): Lst {
  if ($i === $len) {
    return $list_acc;
  }
  $elt = $vec[$i];
  $list_acc = tuple($elt, $list_acc);
  return rec_vec_to_list_tail_rec($vec, $len, $i + 1, $list_acc);
}

function rev_vec_to_list_tail(vec<int> $vec): Lst {
  $len = vec_length($vec);
  return rec_vec_to_list_tail_rec($vec, $len, 0, null);
}

function list_to_vec(Lst $list): vec<int> {
  $vec = vec[];
  while ($list is (mixed, mixed)) {
    list($elt, $list) = $list;
    $vec[] = $elt;
  }
  return $vec;
}

function rev_list_to_vec_rec(Lst $list): vec<int> {
  if ($list is (mixed, mixed)) {
    list($elt, $list) = $list;
    $vec = rev_list_to_vec_rec($list);
    $vec[] = $elt;
    return $vec;
  } else {
    return vec[];
  }
}

function list_to_vec_tail_rec(Lst $list, vec<int> $vec_acc): vec<int> {
  if ($list is (mixed, mixed)) {
    list($elt, $list) = $list;
    $vec_acc[] = $elt;
    return list_to_vec_tail_rec($list, $vec_acc);
  } else {
    return $vec_acc;
  }
}

function list_to_vec_tail(Lst $list): vec<int> {
  return list_to_vec_tail_rec($list, vec[]);
}

function vec_length(vec<int> $vec): int {
  $len = 0;
  foreach ($vec as $_) {
    $len += 1;
  }
  return $len;
}

function vec_equal(vec<int> $vec1, vec<int> $vec2): bool {
  if (vec_length($vec1) !== vec_length($vec2)) {
    return false;
  }
  foreach ($vec1 as $i => $elt) {
    if ($elt !== $vec2[$i]) {
      return false;
    }
  }
  return true;
}

function assert_vec_equal(vec<int> $vec, vec<int> $expected): void {
  if (!vec_equal($expected, $vec)) {
    echo "FAIL got\n";
    var_dump($vec);
  } else {
    echo "PASS\n";
  }
}

<<__EntryPoint>>
function main(): void {
  $vec = vec[1, 2, 3];
  $res = rev_list_to_vec_rec(rev_vec_to_list($vec));
  assert_vec_equal($res, $vec);
  $res = list_to_vec(vec_to_list($vec));
  assert_vec_equal($res, $vec);
  $res = list_to_vec_tail(vec_to_list($vec));
  assert_vec_equal($res, $vec);
  $res = rev_list_to_vec_rec(rev_vec_to_list_tail($vec));
  assert_vec_equal($res, $vec);
}
