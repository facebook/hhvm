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
  <<__Rx, __OnlyRxIfImpl(IRxValue::class)>>
  public function get(): IObj;
}

interface IRxValue extends IValue {
  <<__Rx>>
  public function get(): IRxObj;
}

<<__Rx, __AtMostRxAsArgs>>
function f(<<__OnlyRxIfImpl(IRxValue::class)>>IValue $v): int {
  $obj = $v->get();
  return $obj->foo();
}
