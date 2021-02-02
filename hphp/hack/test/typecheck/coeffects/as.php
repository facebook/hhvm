<?hh

class C {
  const ctx C1 = [];
  const ctx C2 = [write_props];
  const ctx C3 = [defaults, cipp];

  public function f(): void {
    3 as this::C1;
    4 as this::C2;
    5 as this::C3;
  }

  public function g(): void {
    enf<this::C1>(3);
    enf<this::C2>(4);
    enf<this::C3>(5);
  }
}

function f(): void {
  3 as C::C1;
  4 as C::C2;
  5 as C::C3;
}

function enf<<<__Enforceable>> reify T>(mixed $m): T {
  return (4 as T);
}

function g(): void {
  enf<C::C1>(3);
  enf<C::C2>(4);
  enf<C::C3>(5);
}
