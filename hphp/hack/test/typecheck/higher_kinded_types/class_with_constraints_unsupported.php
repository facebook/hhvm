<?hh // strict

class C<T as num> {}

class D<T<TX>> {}

// For now we just reject C generally as a HK type because it has bounds,
// until we can check those bounds properly for subkinding checks
function test(D<C> $x) : void {}
