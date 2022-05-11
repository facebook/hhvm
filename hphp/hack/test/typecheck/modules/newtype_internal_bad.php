<?hh
<<file:__EnableUnstableFeatures("modules")>>
new module foo {}
module foo;
internal class FooInternal {}
newtype FooAlias = FooInternal;
newtype FooAliasWrong as FooInternal = FooInternal;
class EntFoo {
  const type TValue = FooAlias;
}
