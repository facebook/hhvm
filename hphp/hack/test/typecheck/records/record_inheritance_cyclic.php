<?hh

abstract record A extends B {}

abstract record B extends A {}

// Ensure we don't get an infinite loop when there's an inheritance cycle
// earlier in the chain.
abstract record C extends A {}

abstract record D extends D {}
