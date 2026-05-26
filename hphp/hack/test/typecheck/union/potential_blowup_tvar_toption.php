<?hh

function make_tvaropt<T>(): ?T {
  return null;
}

// This test mimicks something found in www
function test(bool $cond, bool $b, int $e): void {
  $a = 0;
  $b = true;
  $c = 1;
  $d = false;
  $e = vec[];

  while ($cond) {
    // a dependency chain to typecheck this loop block multiple times
    $a = $b;
    $b = $c;
    $c = $d;
    $d = $e;

    if ($cond) {
      $x = make_tvaropt();
    } else if ($cond) {
      $x = null;
    } else if ($cond) {
      $x = make_tvaropt();
    } else {
      $x = null;
    }

    if ($cond) {
      return;
    }

    if ($cond) {
      $x = make_tvaropt();
    } else {
      $x = null;
    }

    if ($cond) {
      return;
    }

    if ($cond) {
      $x = make_tvaropt();
    } else if ($cond) {
      $x = null;
    } else if ($cond) {
      $x = make_tvaropt();
    } else if ($cond) {
      $x = null;
    } else if ($cond) {
      $x = make_tvaropt();
    } else if ($cond) {
      $x = null;
    } else if ($cond) {
      $x = make_tvaropt();
    } else {
      $x = null;
    }
    if ($cond) {
      return;
    }
  }
}

function expect_int(int $x): void {}
