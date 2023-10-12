<?hh // strict

enum E: int {}

type MyAlias2 = E;
type MyAlias = MyAlias2;

interface I {}

function test(
  TypeStructure<E> $x,
  TypeStructure<I> $y,
  TypeStructure<MyAlias> $z,
): void {
  hh_show($x['classname']);
  hh_show($y['classname']);
  hh_show($z['classname']);
}
