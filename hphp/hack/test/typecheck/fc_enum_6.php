<?hh // strict

// Should fail because string isn't a supertype of int
enum Foo: int as string {
  FOO = 0;
}
