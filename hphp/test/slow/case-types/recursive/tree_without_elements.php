<?hh
<<file: __EnableUnstableFeatures('case_types')>>

case type A = vec<A>;

function make_A_rec(int $depth): A {
  if ($depth === 0) {
    return vec[];
  }
  return vec[make_A_rec($depth - 1)];
}

function make_A_rec2(A $a, int $depth): A {
  if ($depth > 0) {
    return make_A_rec2(vec[$a], $depth - 1);
  } else {
    return $a;
  }
}

function make_A(int $depth): A {
  return make_A_rec2(vec[], $depth);
}

function is_empty(A $a): bool {
  if ($a is vec<_>) {
    foreach ($a as $_) {
      return false;
    }
    return true;
  } else {
    return true;
  }
}

function depth(A $a): int {
  if ($a is vec<_>) {
    if (is_empty($a)) {
      return 0;
    } else {
      return 1 + depth($a[0]);
    }
  } else {
    return 0;
  }
}

function depth2(A $a): int {
  if ($a is vec<_>) {
    if (is_empty($a)) {
      return 0;
    } else {
      $depth = 0;
      foreach ($a as $a1) {
        $elt_depth = depth2($a1);
        if ($elt_depth > $depth) {
          $depth = $elt_depth;
        }
      }
      return 1 + $depth;
    }
  } else {
    return 0;
  }
}

<<__EntryPoint>>
function main(): void {
  $a = make_A(10);
  $depth = depth($a);
  echo "depth = $depth\n";
  $a = make_A_rec(11);
  $depth2 = depth2($a);
  echo "depth2 = $depth2\n";
}
