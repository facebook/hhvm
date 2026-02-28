<?hh
<<file: __EnableUnstableFeatures('case_types')>>
enum E : string as string {
  A = 'A';
}
type Tstr = E;
final class Foo {}
case type EOrFoo = Tstr | Foo;
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
