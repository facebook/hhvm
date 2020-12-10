<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

// ERROR
<<__Rx>>
function f(<<__AtMostRxAsFunc>>string $a): void {
}
