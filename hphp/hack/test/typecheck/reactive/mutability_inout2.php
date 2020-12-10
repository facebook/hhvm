<?hh
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class A {}

<<__Rx>>
function f(<<__MaybeMutable>> inout A $a): void {
}
