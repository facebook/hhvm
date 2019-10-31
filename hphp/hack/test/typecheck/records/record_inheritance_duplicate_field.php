<?hh

abstract record A {
  x: int,
}

// Direct inheritance should give an error on duplicate fields
record B extends A {
  y: string,
  x: int,
}

// Duplicate fields may occur higher in the inheritance chain.
abstract record BB extends A {}

record CC extends BB {
  x: int,
}
