<?hh

module foo;

type TFoo1 = int;
newtype TFoo2 = string;
interface IFoo<T> {}
class Foo {}
function foo(): mixed {
  echo "I'm in foo\n";
}
enum EFoo: int {
  ZERO = 0;
  ONE = 1;
}
final class CFoo implements HH\ClassAttribute {}
