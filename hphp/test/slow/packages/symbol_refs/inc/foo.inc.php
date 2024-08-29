<?hh

module foo;

type TFoo1 = int;
newtype TFoo2 = string;
interface IFoo<T> {}
class Foo {}
function foo(): mixed {
  echo "I'm in foo\n";
}
function getFoo(): Foo {
  return new Foo();
}
