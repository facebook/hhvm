<?hh // strict

enum Foo : int as int {
  FOO = 1;
  BAR = 2;
  BAZ = 3;
}

enum Bar : Foo as int {
  FOO = Foo::FOO;
  BAR = Foo::BAR;
}


function test(@Bar $x): void {
  var_dump($x);
}

// These should be fine
test(Bar::FOO);
test(Bar::BAR);
// These should fail
test("hello");
test(10.0);
