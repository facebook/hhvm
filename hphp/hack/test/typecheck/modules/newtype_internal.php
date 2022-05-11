<?hh
<<file:__EnableUnstableFeatures("modules")>>
new module foo {}
module foo;
internal class FooInternal {}
newtype FooAlias = FooInternal;

class EntFoo {
  const type TValue = FooAlias;
}
