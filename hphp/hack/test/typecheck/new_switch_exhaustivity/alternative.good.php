<?hh

enum E: int { A = 1; }
enum F: int { A = 1; }
function main((E & F) $e): void {
  // The following is exhaustive
  switch ($e) {
    case E::A:
      break;
  }
  // So is the following
  switch ($e) {
    case F::A:
      break;
  }
}
