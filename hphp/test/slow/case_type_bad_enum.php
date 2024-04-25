<?hh
<<file: __EnableUnstableFeatures('case_types')>>

class A {}
class Foo {}

newtype Alias = A;

enum E : A {
  A = 'A';
}

case type EorFoo = E | Foo | int | string;

function take(EOrFoo $e_or_foo): void {}
function test(bool $which): void {
  take($which ? new Foo() : E::A);
}
<<__EntryPoint>>
function main(): void {
  test(false);
  test(true);
  echo "OK\n";
}
