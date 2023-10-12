<?hh // strict

function make_tvaropt<T>(): ?T {
  return null;
}

// This test mimicks something found in www
function test(bool $b, int $e): void {
  $a = 0;
  $b = true;
  $c = 1;
  $d = false;
  $e = vec[];

  while ($b) {
    // a dependency chain to typecheck this loop block multiple times
    $a = $b;
    $b = $c;
    $c = $d;
    $d = $e;

    if ($b) {
      $x = make_tvaropt();
    } else if ($b) {
      $x = null;
    } else if ($b) {
      $x = make_tvaropt();
    } else {
      $x = null;
    }

    if ($b) {
      return;
    }

    if ($b) {
      $x = make_tvaropt();
    } else {
      $x = null;
    }

    if ($b) {
      return;
    }

    if ($b) {
      $x = make_tvaropt();
    } else if ($b) {
      $x = null;
    } else if ($b) {
      $x = make_tvaropt();
    } else if ($b) {
      $x = null;
    } else if ($b) {
      $x = make_tvaropt();
    } else if ($b) {
      $x = null;
    } else if ($b) {
      $x = make_tvaropt();
    } else {
      $x = null;
    }
    if ($b) {
      return;
    }
  }
}

function expect_int(int $x): void {}
