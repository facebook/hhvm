<?hh
<<file: __EnableUnstableFeatures('case_types')>>

case type Forest<T> = (Tree<T>, Forest<T>) | null;
case type Tree<T> = shape(
    'tree' => T,
    'forest' => Forest<T>,
  );

function tree_data<T>(Tree<T> $tree): T {
  if ($tree is shape(...)) {
    return $tree['tree'];
  }
}

case type Lst<T> = (T, Lst<T>) | null;

function make<T>(vec<T> $trees): Forest<T> {
  $forest = null;
  foreach ($trees as $tree) {
    $forest = tuple(shape('tree' => $tree, 'forest' => $forest), $forest);
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
    return tuple($tree, $forest);
  }
}

function make_tail_rec<T>(Lst<T> $trees, Forest<T> $forest_acc): Forest<T> {
  if ($trees is null) {
    return $forest_acc;
  } else {
    list($tree_data, $trees) = $trees;
    $tree = shape('tree' => $tree_data, 'forest' => $forest_acc);
    $forest_acc = tuple($tree, $forest_acc);
    return make_tail_rec($trees, $forest_acc);
  }
}

function to_list<T>(Forest<T> $forest): Lst<T> {
  if ($forest is null) {
    return null;
  } else {
    list($tree, $forest) = $forest;
    $tree_data = tree_data($tree);
    $forest = to_list($forest);
    return tuple($tree_data, $forest);
  }
}

function to_list_tail_rec<T>(Forest<T> $forest, Lst<T> $list_acc): Lst<T> {
  if ($forest is null) {
    return $list_acc;
  } else {
    list($tree, $forest) = $forest;
    $tree_data = tree_data($tree);
    $list_acc = tuple($tree_data, $list_acc);
    return to_list_tail_rec($forest, $list_acc);
  }
}

function to_vec<T>(Forest<T> $forest): vec<T> {
  $result = vec[];
  while ($forest is nonnull) {
    list($tree, $forest) = $forest;
    $tree_data = tree_data($tree);
    $result[] = $tree_data;
  }
  return $result;
}

<<__EntryPoint>>
function main() {
  $trees = vec[1, 2, 3];
  $forest = make($trees);
  $trees2 = to_vec($forest);
  var_dump($trees2);
  $forest = make_rec(to_list($forest));
  var_dump(to_vec($forest));
  $forest = make_tail_rec(to_list($forest), null);
  var_dump(to_vec($forest));
  $forest = make_rec(to_list_tail_rec($forest, null));
  var_dump(to_vec($forest));
  $forest = make_tail_rec(to_list_tail_rec($forest, null), null);
  var_dump(to_vec($forest));
}
