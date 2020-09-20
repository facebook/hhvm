<?hh

// Test that typechecks work when the underlying type is a typedef
type Foo = int;
enum Bar : Foo as Foo {
  FOO = 1;
}

function test(<<__Soft>> Bar $x): void {
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
