<?hh
<<file:__EnableUnstableFeatures("readonly")>>

class Foo {}
function foo(readonly Foo ...$x) : void {}
