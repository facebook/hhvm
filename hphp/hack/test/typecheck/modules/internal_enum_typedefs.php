<?hh
<<file:__EnableUnstableFeatures("modules"), __Module("foo")>>
new module foo {}

internal newtype X = int;

internal enum Foo : int {
  HELLO = 1;

}
