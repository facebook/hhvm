<?hh

enum E: int {
  A = 0;
  B = 1;
}

function test_like_null(~null $n): void {
  $n = id<~null>($n);
  switch ($n) {
    case null:
      return;
  }
}

function test_nullable_dynamic(?dynamic $n): void {
  $n = id<?dynamic>($n);
  switch ($n) {
    case null:
      return;
  }
}

function test_nullable_enum_without_null(?E $e): void {
  switch ($e) {
    case E::A:
      return;
    case E::B:
      return;
  }
}

function test_nullable_enum_without_enum(?E $e): void {
  switch ($e) {
    case null:
      return;
  }
}

function test_like_nullable_enum(~?E $e): void {
  $e = id<~?E>($e);
  switch ($e) {
    case E::A:
      return;
    case E::B:
      return;
  }
}

function test_nullable_like_enum(?~E $e): void {
  $e = id<?~E>($e);
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
