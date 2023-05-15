<?hh

enum E: int {
  A = 42;
  B = 43;
}

function main(E $e): void {
  switch ($e) {
    case E::A:
      echo "E::A";
      break;
    case E::B:
      echo "E::B";
      break;
    default:
      echo "default";
  }
}
