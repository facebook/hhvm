<?hh

enum Foo : int as int {
  FOO = 1;
  BAR = 2;
  BAZ = 3;
}

enum Bar : Foo as int {
  FOO = Foo::FOO;
  BAR = Foo::BAR;
}


function test(<<__Soft>> Bar $x): void {
  var_dump($x);
}
<<__EntryPoint>> function main(): void {
// These should be fine
test(Bar::FOO);
test(Bar::BAR);
// These should fail
test("hello");
test(10.0);
}
