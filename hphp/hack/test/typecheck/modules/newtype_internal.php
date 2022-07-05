//// modules.php
<?hh
<<file:__EnableUnstableFeatures("modules")>>

new module foo {}

//// test.php
<?hh
<<file:__EnableUnstableFeatures("modules")>>
module foo;
internal class FooInternal {}
newtype FooAlias = FooInternal;

class EntFoo {
  const type TValue = FooAlias;
}
