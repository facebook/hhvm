<?hh // strict

// If a class is declared abstract final then it cannot contain any abstract
// type constants

interface I {
  abstract const type T;
}

abstract final class C implements I {}
