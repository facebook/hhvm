<?hh

abstract class A {
  abstract const type T;

  // returns refined A, or A and empty body?
  public abstract function ambiguous_parse(): A {} ;
}

type TAlias = A { type T = int };
