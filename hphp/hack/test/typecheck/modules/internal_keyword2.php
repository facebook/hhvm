<?hh
<<file:__EnableUnstableFeatures("modules"), __Module("foo")>>
module foo {}
class Foo {
  public internal function bar(): void {}
}
