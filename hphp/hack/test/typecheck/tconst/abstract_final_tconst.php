<?hh

// If a class is declared abstract final then it cannot contain any abstract
// type constants

abstract final class C {
  abstract const type T;
}
