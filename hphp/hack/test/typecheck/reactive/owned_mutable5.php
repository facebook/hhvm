<?hh
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

// ERROR
function f(<<__OwnedMutable>> A $a): void {
}

interface A {}
