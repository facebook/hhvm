<?hh

enum E: int {
  A = 0;
  B = 1;
}

enum F: int {
  A = 0;
  B = 1;
}

function test_nullable_as_union((E | null) $e): void {
  switch ($e) {
    case E::A:
      return;
    case E::B:
      return;
  }
}

function test_like_nullable_as_union(~(E | null) $e): void {
  $e = id<~(E | null)>($e);
  switch ($e) {
    case E::A:
      return;
    case null:
      return;
  }
}

function test_nullable_as_union_like1((~E | null) $e): void {
  $e = id<(~E | null)>($e);
  switch ($e) {
    case E::B:
      return;
    case null:
      return;
  }
}

function test_nullable_as_union_like2((E | ~null) $e): void {
  $e = id<(E | ~null)>($e);
  switch ($e) {
    case E::A:
      return;
    case E::B:
      return;
  }
}

function test_nullable_as_union_like3((~E | ~null) $e): void {
  $e = id<(~E | ~null)>($e);
  switch ($e) {
    case E::A:
      return;
    case null:
      return;
  }
}

function test_enum_or_enum((E | F) $e): void {
  switch ($e) {
    case E::A:
      return;
    case E::B:
      return;
    case F::A:
      return;
  }
}

function test_enum_or_same_enum((E | E) $e): void {
  switch ($e) {
    case E::A:
      return;
  }
}

<<__NoAutoLikes>>
function id<T>(T $x): T {
  return $x;
}
