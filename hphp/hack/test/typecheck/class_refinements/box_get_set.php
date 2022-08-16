<?hh

<<file:__EnableUnstableFeatures('type_refinements')>>

abstract class Box {
  abstract const type T as mixed;
  abstract public function get() : this::T;
  abstract public function set(this::T $val) : void;
}

function legacy_get<Tb as Box, T>(Tb $b) : T where T = Tb::T {
  return $b->get();
}

function tyref_get<T>(bool $rnd, Box with {type T = T} $b): T {
  if ($rnd) {
    return $b->get();
  } else {
    return legacy_get($b);
  }
}

function tyref_set<T>(Box with {type T = T} $b, T $val): void {
  $b->set($val);
}

class IntBox extends Box {
  const type T = int;
  public function get(): int { return 42; }
  public function set(int $x): void { }
}

function expect<T>(T $x): void {}

function test_ok(): void {
  $ib = new IntBox();
  expect<int>(tyref_get(true, $ib));
  tyref_set($ib, 10);
}

function test_ko(): void {
  $ib = new IntBox();
  // Error below:
  expect<string>(tyref_get(true, $ib));
  // Error below:
  tyref_set($ib, "hi");
}
