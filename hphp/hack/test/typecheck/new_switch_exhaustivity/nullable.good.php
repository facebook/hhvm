<?hh

enum E: int {
  A = 0;
  B = 1;
}

function test_null(): void {
  switch (null) {
    case null:
      return;
  }
}

function test_nullable_enum(?E $e): void {
  switch ($e) {
    case E::A:
      return;
    case E::B:
      return;
    case null:
      return;
  }
}

function test_nullable_enum_with_default(?E $e): void {
  switch ($e) {
    case E::A:
      return;
    case E::B:
      return;
    default:
      return;
  }
}

function test_nullable_enum_with_default2(?E $e): void {
  switch ($e) {
    case null:
      return;
    default:
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
    case null:
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
    case null:
      return;
  }
}

<<__NoAutoLikes>>
function id<T>(T $x): T {
  return $x;
}
