<?hh // strict

interface IObj {
  <<__Rx, __OnlyRxIfImpl(IFakeRxObj::class)>>
  public function foo(): int;
}

interface IFakeRxObj {
  public function foo(): int;
}

interface IValue {
  <<__Rx, __OnlyRxIfImpl(IRxValue::class)>>
  public function get(): IObj;
}

interface IRxValue {
  <<__Rx>>
  public function get(): IFakeRxObj;
}

<<__Rx, __AtMostRxAsArgs>>
function f(<<__OnlyRxIfImpl(IRxValue::class)>>IValue $v): int {
  $obj = $v->get();
  // ERROR: foo in IFakeRxObj is non-reactive
  return $obj->foo();
}
