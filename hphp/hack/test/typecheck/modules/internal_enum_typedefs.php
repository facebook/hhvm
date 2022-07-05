//// modules.php
<?hh
<<file:__EnableUnstableFeatures("modules")>>

new module foo {}

//// test.php
<?hh
<<file:__EnableUnstableFeatures("modules")>>
module foo;

internal newtype X = int;

internal enum Foo : int {
  HELLO = 1;

}
