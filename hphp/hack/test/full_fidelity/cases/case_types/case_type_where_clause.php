<?hh

interface MyInt {}
interface MyString {}
interface MyOther {}

case type CTSingleVariant<T> = int where T super MyInt;

case type CTwoVariants<T> = int where T super MyInt | string where T super MyString;

case type CTwoVariantsOneWithoutWhere<T> = int where T super MyInt | string;

case type CMultipleConstraints<Tu, Tv> =
  | int where Tu super MyInt, Tv as MyOther
  | string where Tu super MyString;

function foo(string $b): CMultipleConstraints<MyString, nothing> {
  return $b;
}
