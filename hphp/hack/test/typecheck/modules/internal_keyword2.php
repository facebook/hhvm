//// modules.php
<?hh
<<file:__EnableUnstableFeatures("modules")>>

new module foo {}

//// test.php
<?hh
<<file:__EnableUnstableFeatures("modules")>>
module foo;
class Foo {
  public internal function bar(): void {}
}
