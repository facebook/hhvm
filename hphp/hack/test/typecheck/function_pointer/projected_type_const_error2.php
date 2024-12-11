<?hh

abstract class MyBox {
  abstract const type TInner;
  abstract public function set(this::TInner $_): void;
}

class IntBox extends MyBox {
  const type TInner = int;
  public function set(int $_): void {}
}

function set_a_box<TBox as MyBox with { type TInner = TVal }, TVal>(
  TBox $b,
  TVal $v,
): void {
  $b->set($v);
}

function test(): HH\FunctionRef<(readonly function(IntBox, string): void)> {
  return set_a_box<>;
}
