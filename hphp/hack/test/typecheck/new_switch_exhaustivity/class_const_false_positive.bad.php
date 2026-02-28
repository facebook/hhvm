<?hh

enum E: string {
  A = "a";
  B = "b";
}

class C {
  const E B = E::B;
}

function main(E $e): void {
  // This switch is exhaustive, but we can't see C::B = E::B, so we require a
  // default here.
  switch ($e) {
    case E::A: break;
    case C::B: break;
  }
}
