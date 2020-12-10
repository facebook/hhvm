<?hh
<<file: __EnableUnstableFeatures('coeffects_provisional')>>
interface Rx {}
interface A {}

<<__Rx, __AtMostRxAsArgs>>
function f(<<__OnlyRxIfImpl(Rx::class)>>?A $a = null): void {
}
