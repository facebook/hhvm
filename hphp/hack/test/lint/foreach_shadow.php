<?hh

function f1(array<int> $xs): int {
  $x = 0;
  foreach ($xs as $x) {
    $x++;
  }
  return $x;
}

function f2(array<int> $xs): int {
  $x = 1;
  $x = $x - 1;
  foreach ($xs as $x) {
    $x++;
  }
  return $x;
}

function f3(array<int, int> $xs): int {
  $x = 0;
  foreach ($xs as $x => $_) {
    $x++;
  }
  return $x;
}

function f4(array<int, int> $xs): int {
  $x = 0;
  foreach ($xs as $_ => $x) {
    $x++;
  }
  return $x;
}

function f5(array<int, (int, int)> $xs): int {
  $x = 0;
  foreach ($xs as $_ => list($x, $_)) {
    $x++;
  }
  return $x;
}

function f6(array<int> $xs): int {
  $x = 0;
  if (true) {
    foreach ($xs as $x) {
      $x++;
    }
  }
  return $x;
}

function f7(array<array<int>> $xss): int {
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

function f8(array<int> $xs): array<(int, int)> {
  $result = varray[];
  foreach ($xs as $x) {
    $x1 = $x;
    foreach ($xs as $x) {
      $x2 = $x;
      $result[] = tuple($x1, $x2);
    }
  }
  return $result;
}

function f9(array<int> $xs): int {
  list($x, $_) = varray[0, 0];
  foreach ($xs as $x) {
    $x++;
  }
  return $x;
}

function f10(array<int> $xs): int {
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

function f11(array<int> $xs): int {
  $s = 0;
  // The loop variable $x doesn't shadow $x defined after the loop.
  foreach ($xs as $x) {
    $s += $x;
  }
  $x = 0;
  $s += $x;
  return $s;
}

function f12(array<int> $xs): int {
  if (false) {
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

function f13(array<int, array<int>> $xss): int {
  $s = 0;
  foreach ($xss as $_ => $xs) {
    // Variables named $_ are ignored
    foreach ($xs as $_) {
      $s += 1;
    }
  }
  return $s;
}
