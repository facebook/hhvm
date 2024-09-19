<?hh
<<file: __EnableUnstableFeatures('case_types')>>

case type Lst<T> = (T, Lst<T>) | null;

function reverse_list<T>(Lst<T> $list): Lst<T> {
  $list_acc = null;
  while ($list is nonnull) {
    list($v, $list) = $list;
    $list_acc = tuple($v, $list_acc);
  }
  return $list_acc;
}

function rev_vec_to_list<T>(vec<T> $vec): Lst<T> {
  $list = null;
  foreach ($vec as $v) {
    $list = tuple($v, $list);
  }
  return $list;
}

function list_to_vec<T>(Lst<T> $list): vec<T> {
  $vec = vec[];
  while ($list is nonnull) {
    list($v, $list) = $list;
    $vec[] = $v;
  }
  return $vec;
}

case type Forest<T> = Tree<T> | null;
type Tree<T> = shape(
  'tree' => T,
  'forest' => Forest<T>,
);

function rev_make<T>(vec<T> $trees): Forest<T> {
  $forest = null;
  foreach ($trees as $tree) {
    $forest = shape('tree' => $tree, 'forest' => $forest);
  }
  return $forest;
}

function make_rec<T>(Lst<T> $trees): Forest<T> {
  if ($trees is null) {
    return null;
  } else {
    list($tree_data, $trees) = $trees;
    $forest = make_rec($trees);
    $tree = shape('tree' => $tree_data, 'forest' => $forest);
    return $tree;
  }
}

function rev_make_tail_rec<T>(Lst<T> $trees, Forest<T> $forest_acc): Forest<T> {
  if ($trees is null) {
    return $forest_acc;
  } else {
    list($tree_data, $trees) = $trees;
    $forest_acc = shape('tree' => $tree_data, 'forest' => $forest_acc);
    return rev_make_tail_rec($trees, $forest_acc);
  }
}

function to_list_rec<T>(Forest<T> $forest): Lst<T> {
  if ($forest is null) {
    return null;
  } else {
    $tree_data = $forest['tree'];
    $forest = to_list_rec($forest['forest']);
    return tuple($tree_data, $forest);
  }
}

function rev_to_list_tail_rec<T>(Forest<T> $forest, Lst<T> $list_acc): Lst<T> {
  if ($forest is null) {
    return $list_acc;
  } else {
    $tree_data = $forest['tree'];
    $list_acc = tuple($tree_data, $list_acc);
    return rev_to_list_tail_rec($forest['forest'], $list_acc);
  }
}

function reverse_forest_tail_rec<T>(
  Forest<T> $forest,
  Forest<T> $forest_acc,
): Forest<T> {
  if ($forest is null) {
    return $forest_acc;
  } else {
    $tree = $forest['tree'];
    $forest = $forest['forest'];
    $forest_acc = shape('tree' => $tree, 'forest' => $forest_acc);
    return reverse_forest_tail_rec($forest, $forest_acc);
  }
}

function reverse_forest_tail_rec2<T>(
  Forest<T> $forest,
  Forest<T> $forest_acc,
): Forest<T> {
  if ($forest is shape(...)) {
    $tree = $forest['tree'];
    $forest = $forest['forest'];
    $forest_acc = shape('tree' => $tree, 'forest' => $forest_acc);
    return reverse_forest_tail_rec2($forest, $forest_acc);
  } else {
    return $forest_acc;
  }
}

function reverse_forest<T>(Forest<T> $forest): Forest<T> {
  return reverse_forest_tail_rec2($forest, null);
}

function vec_length<T>(vec<T> $vec): int {
  $len = 0;
  foreach ($vec as $_) {
    $len += 1;
  }
  return $len;
}

function vec_equal<T>(vec<T> $vec1, vec<T> $vec2): bool {
  if (vec_length($vec1) !== vec_length($vec2)) {
    return false;
  }
  foreach ($vec1 as $i => $v) {
    if ($v !== $vec2[$i]) {
      return false;
    }
  }
  return true;
}

function test_list_is<T>(Lst<T> $list, vec<T> $expected): void {
  $vec = list_to_vec($list);
  if (!vec_equal($expected, $vec)) {
    echo "FAIL got\n";
    var_dump($vec);
  }
}

function test_to_list<T>(Forest<T> $forest, vec<T> $expected): void {
  $list = to_list_rec($forest);
  test_list_is($list, $expected);
  $list = rev_to_list_tail_rec($forest, null);
  test_list_is(reverse_list($list), $expected);
}

<<__EntryPoint>>
function main(): void {
  $vec = vec[1, 2, 3];
  $forest = reverse_forest(rev_make($vec));
  test_to_list($forest, $vec);
  $forest = reverse_forest(make_rec(rev_vec_to_list($vec)));
  test_to_list($forest, $vec);
  $forest = rev_make_tail_rec(rev_vec_to_list($vec), null);
  test_to_list($forest, $vec);
}
