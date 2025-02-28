<?hh

abstract class BaseClassA {}
abstract class BaseClassB {}

interface IFoo {}

interface IForWhereClause {}

case type MyCaseType<+T> =
  | BaseClassA where T super IForWhereClause
  | BaseClassB
  | null;

function test<T>(MyCaseType<T> $x): MyCaseType<T> {
  if ($x is IFoo) {
  }
  return $x;
}
