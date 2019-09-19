<?hh // strict
class NonRxClass {}

interface IRxClass {}

<<__RxLocal, __AtMostRxAsArgs>>
function maybe_reactive1(
  <<__OnlyRxIfImpl(IRxClass::class)>> ?NonRxClass $cls = null,
): void {}

<<__Rx>>
function break_reactivity(): void {
  maybe_reactive1();
}
