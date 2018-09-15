<?hh // strict

// ERROR, __OnlyRxIfRxFunc can only appear on parameters of
// conditionally reactive functions
<<__OnlyRxIfRxFunc>>
function f(): void {
}
