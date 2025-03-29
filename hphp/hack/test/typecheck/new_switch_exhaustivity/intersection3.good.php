<?hh

enum E: int {
  A = 0;
}

function f(((nonnull & dynamic) | (int & dynamic) | E) $e): void {
  switch ($e) {
    case E::A:
      return;
  }
}
