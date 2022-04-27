<?hh
<<file:__EnableUnstableFeatures("modules")>>
module foo;
new module foo {}

internal newtype X = int;

internal enum Foo : int {
  HELLO = 1;

}
