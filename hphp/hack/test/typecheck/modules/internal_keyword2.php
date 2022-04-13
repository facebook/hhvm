<?hh
<<file:__EnableUnstableFeatures("modules"), __Module("foo")>>
new module foo {}
class Foo {
  public internal function bar(): void {}
}
