<?hh

enum Foo : int {
  FOO = 1;
  BAR = 2;
  BAZ = 3;
}

enum Bar : Foo as Foo {
  FOO = Foo::FOO;
}

// Nullable enum supertype is still a supertype
function f(): ?Bar {
  return Bar::FOO;
}
