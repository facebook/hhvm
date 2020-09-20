<?hh // strict

interface IObj {
  <<__Rx, __OnlyRxIfImpl(IRxObj::class)>>
  public function foo(): int;
}

interface IRxObj extends IObj {
  <<__Rx>>
  public function foo(): int;
}

interface IValue {
  <<__Rx, __OnlyRxIfImpl(IFakeRxValue::class)>>
  public function get(): IObj;
}

interface IFakeRxValue {
  public function get(): IRxObj;
}

<<__Rx, __AtMostRxAsArgs>>
function f(<<__OnlyRxIfImpl(IFakeRxValue::class)>>IValue $v): int {
  // ERROR: get in IFakeRxValue is non-reactive
  $obj = $v->get();
  return $obj->foo();
}
