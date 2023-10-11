<?hh

<<__ConsistentConstruct>>
class C {}

type CAlias = C;

function new_c(classname<CAlias> $c): CAlias {
  return new $c();
}

function bad_new_c(typename<CAlias> $c): CAlias {
  return new $c();
}
