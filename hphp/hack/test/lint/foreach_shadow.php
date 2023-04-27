<?hh

function f1(vec<int> $xs): int {
  $x = 0;
  foreach ($xs as $x) {
    $x++;
  }
  return $x;
}

function f2(vec<int> $xs): int {
  $x = 1;
  $x = $x - 1;
  foreach ($xs as $x) {
    $x++;
  }
  return $x;
}

function f3(dict<int, int> $xs): int {
  $x = 0;
  foreach ($xs as $x => $_) {
    $x++;
  }
  return $x;
}

function f4(dict<int, int> $xs): int {
  $x = 0;
  foreach ($xs as $_ => $x) {
    $x++;
  }
  return $x;
}

function f5(dict<int, (int, int)> $xs): int {
  $x = 0;
  foreach ($xs as $_ => list($x, $_)) {
    $x++;
  }
  return $x;
}

function f6(vec<int> $xs): int {
  $x = 0;
  if (true) {
    foreach ($xs as $x) {
      $x++;
    }
  }
  return $x;
}

function f7(vec<vec<int>> $xss): int {
  $s = 0;
  foreach ($xss as $xs) {
    $x = 0;
    foreach ($xs as $x) {
      $x++;
    }
    $s += $x;
  }
  return $x;
}

function f8(vec<int> $xs): vec<(int, int)> {
  $result = vec[];
  foreach ($xs as $x) {
    $x1 = $x;
    foreach ($xs as $x) {
      $x2 = $x;
      $result[] = tuple($x1, $x2);
    }
  }
  return $result;
}

function f9(vec<int> $xs): int {
  list($x, $_) = vec[0, 0];
  foreach ($xs as $x) {
    $x++;
  }
  return $x;
}

function f10(vec<int> $xs): int {
  $s = 0;
  try {
    throw new Exception();
  } catch (Exception $x) {
    echo $x->getMessage();
    foreach ($xs as $x) {
      $s += $x;
    }
  }
  return $s;
}

function f11(vec<int> $xs): int {
  $s = 0;
  // The loop variable $x doesn't shadow $x defined after the loop.
  foreach ($xs as $x) {
    $s += $x;
  }
  $x = 0;
  $s += $x;
  return $s;
}

function f12(vec<int> $xs): int {
  if (1 === 2) {
    $x = 0;
    $s = $x;
  } else {
    $s = 0;
    // The loop variable $x doesn't shadow $x defined in the other
    // branch of if.
    foreach ($xs as $x) {
      $s += $x;
    }
  }
  return $s;
}

function f13(dict<int, vec<int>> $xss): int {
  $s = 0;
  foreach ($xss as $_ => $xs) {
    // Variables named $_ are ignored
    foreach ($xs as $_) {
      $s += 1;
    }
  }
  return $s;
}
