<?hh // strict
class NonRxClass {}

interface IRxClass {}

<<__Rx, __AtMostRxAsArgs>>
function maybe_reactive2(
  <<__OnlyRxIfImpl(IRxClass::class)>> ?NonRxClass $cls = null,
): void {}

<<__Rx>>
function break_reactivity(): void {
  maybe_reactive2();
}
