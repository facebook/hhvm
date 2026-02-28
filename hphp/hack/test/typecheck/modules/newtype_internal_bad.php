//// modules.php
<?hh


new module foo {}

//// test.php
<?hh

module foo;
internal class FooInternal {}
newtype FooAlias = FooInternal;
newtype FooAliasWrong as FooInternal = FooInternal;
class EntFoo {
  const type TValue = FooAlias;
}
