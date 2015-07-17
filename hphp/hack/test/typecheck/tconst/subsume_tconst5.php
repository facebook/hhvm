<?hh

abstract class PartiallyAbstract {
  abstract const type T1;
  const type T2 as mixed = mixed;
}

abstract class FullyAbstract extends PartiallyAbstract {
  abstract const type T2;
}
