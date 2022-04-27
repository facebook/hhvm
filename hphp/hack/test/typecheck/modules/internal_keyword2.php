<?hh
<<file:__EnableUnstableFeatures("modules")>>
module foo;
new module foo {}
class Foo {
  public internal function bar(): void {}
}
