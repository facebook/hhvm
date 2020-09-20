<?hh

// Test that typechecks work when the underlying type is a typedef
enum Bar : int as int {
  FOO = 1;
}
type Foo = Bar;

function test(<<__Soft>> Foo $x): void {
  var_dump($x);
}
<<__EntryPoint>> function main(): void {
// Should be fine
test(Bar::FOO);
// Should produce warning
test("uh");
// Should produce warning
test(1.0);
}
