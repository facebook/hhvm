<?hh

enum E: int {
  A = 42;
}

function foo(?E $e): void {
  if ($e is null) {
    return;
  }

  switch ($e) {
    case E::A:
      echo 'hello';
  }
}
