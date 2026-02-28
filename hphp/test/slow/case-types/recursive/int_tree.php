<?hh
<<file: __EnableUnstableFeatures('case_types')>>

case type A = vec<A> | int;

function make_A_rec(int $depth, int $elt): A {
  if ($depth === 0) {
    return $elt;
  }
  return vec[make_A_rec($depth - 1, $elt)];
}

function make_A_rec2(A $a, int $depth): A {
  if ($depth > 0) {
    return make_A_rec2(vec[$a], $depth - 1);
  } else {
    return $a;
  }
}

function make_A(int $depth, int $elt): A {
  return make_A_rec2($elt, $depth);
}

function is_empty(A $a): bool {
  if ($a is vec<_>) {
    foreach ($a as $_) {
      return false;
    }
    return true;
  } else {
    return false;
  }
}

function depth(A $a): int {
  if ($a is vec<_>) {
    if (is_empty($a)) {
      return 1;
    } else {
      return 1 + depth($a[0]);
    }
  } else {
    return 0;
  }
}

function depth_b(A $a): int {
  if ($a is int) {
    return 0;
  } else {
    if (is_empty($a)) {
      return 1;
    } else {
      return 1 + depth_b($a[0]);
    }
  }
}

function depth2(A $a): int {
  if ($a is vec<_>) {
    $depth = 0;
    foreach ($a as $a1) {
      $elt_depth = depth2($a1);
      if ($elt_depth > $depth) {
        $depth = $elt_depth;
      }
    }
    return 1 + $depth;
  } else {
    return 0;
  }
}

function depth2_b(A $a): int {
  if ($a is int) {
    return 0;
  } else {
    $depth = 0;
    foreach ($a as $a1) {
      $elt_depth = depth2_b($a1);
      if ($elt_depth > $depth) {
        $depth = $elt_depth;
      }
    }
    return 1 + $depth;
  }
}

<<__EntryPoint>>
function main(): void {
  $a = make_A(10, 0);
  $depth = depth($a);
  $depth_b = depth_b($a);
  $depth2 = depth2($a);
  $depth2_b = depth2_b($a);
  echo "depth = $depth\n";
  echo "depth_b = $depth_b\n";
  echo "depth2 = $depth2\n";
  echo "depth2_b = $depth2_b\n";
}
