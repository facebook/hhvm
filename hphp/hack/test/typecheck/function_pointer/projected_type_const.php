<?hh

abstract class MyBox {
  abstract const type TInner;
  abstract public function set(this::TInner $_): void;
}

class IntBox extends MyBox {
  const type TInner = int;
  public function set(int $_): void {}
}

function set_a_box<TBox as MyBox, TVal>(TBox $b, TVal $v): void where TVal = TBox::TInner {
  $b->set($v);
}

function test(): void {
  $fun = set_a_box<>;
  $fun(new IntBox(), 1);
}
