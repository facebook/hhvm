<?hh

abstract record A {
  int x;
}

// Direct inheritance should give an error on duplicate fields
record B extends A {
  string y;
  int x;
}

// Duplicate fields may occur higher in the inheritance chain.
abstract record BB extends A {}

record CC extends BB {
  int x;
}
