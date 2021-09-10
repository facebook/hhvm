<?hh // partial

abstract class HasConcrete {
  abstract const type T1;
  const type T2  = mixed;
}

abstract class FullyAbstract extends HasConcrete {
  abstract const type T2;
}
