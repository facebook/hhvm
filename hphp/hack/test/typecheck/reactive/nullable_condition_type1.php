<?hh
interface Rx {}
interface A {}

<<__Rx, __OnlyRxIfArgs>>
function f(<<__OnlyRxIfImpl(Rx::class)>>?A $a = null) {
}
