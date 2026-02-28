<?hh

enum E: int {
  A = 0;
  B = 1;
}

enum F: int {
  C = 0;
  D = 1;
}

function test_intersection1((E & F) $e): void {
  switch ($e) {
    case E::A:
      return;
    case E::B:
      return;
  }
}

function test_intersection2((E & F) $e): void {
  switch ($e) {
    case F::C:
      return;
    case F::D:
      return;
  }
}

function test_intersection3(~(E & F) $e): void {
  $e = id<~(E & F)>($e);
  switch ($e) {
    case E::A:
      return;
    case E::B:
      return;
  }
}

function test_intersection4((~E & F) $e): void {
  $e = id<(~E & F)>($e);
  switch ($e) {
    case E::A:
      return;
    case E::B:
      return;
  }
}

function test_intersection5((E & ~F) $e): void {
  $e = id<(E & ~F)>($e);
  switch ($e) {
    case E::A:
      return;
    case E::B:
      return;
  }
}

<<__NoAutoLikes>>
function id<T>(T $x): T {
  return $x;
}
