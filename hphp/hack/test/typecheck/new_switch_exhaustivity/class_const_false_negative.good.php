<?hh

enum E: string {
  A = "a";
  B = "b";
}

class C {
  const E B = E::B;
}

function main(E $e): void {
  switch ($e) {
    case E::B: break;
    // This case is redundant but we can't see C::B = E::B
    case C::B: break;
    default:
      break;
  }
}
