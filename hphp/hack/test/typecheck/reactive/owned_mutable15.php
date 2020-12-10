<?hh
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

// ERROR
<<__Rx>>
function f(<<__OwnedMutable, __MaybeMutable>> A $a): void {
}

interface A {}
