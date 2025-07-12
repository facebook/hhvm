<?hh

enum E: int {
  A = 0;
}

function main(shape(?'e' => E, ...) $s): void {
  $e = Shapes::idx($s, 'e');
  if ($e is null) {
    return;
  }
  switch ($e) {
    case E::A:
      break;
  }
}
