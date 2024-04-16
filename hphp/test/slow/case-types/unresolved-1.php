<?hh
<<file: __EnableUnstableFeatures('case_types')>>

final class Foo {}
case type EOrFoo = E | Foo;

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
