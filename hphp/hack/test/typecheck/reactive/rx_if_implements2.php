<?hh // strict

interface Rx1 {}

// ERROR: __RxShallowIfImplements can only be put on methods
<<__RxShallowIfImplements(Rx1::class)>>
function mayberx(): void {
}
