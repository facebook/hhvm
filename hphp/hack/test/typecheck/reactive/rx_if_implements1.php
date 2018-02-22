<?hh // strict

interface Rx1 {}

// ERROR: __RxIfImplements can only be put on methods
<<__RxIfImplements(Rx1::class)>>
function mayberx(): void {
}
