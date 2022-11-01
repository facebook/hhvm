//// def.php
<?hh
<<file:__EnableUnstableFeatures("modules")>>
new module foo {}
//// use.php
<?hh
<<file:__EnableUnstableFeatures("modules")>>
module foo;

public class Foo {
  <<__DynamicallyCallable>>
  internal function foo(): void {}
}
