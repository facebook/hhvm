<?hh

<<file:__EnableUnstableFeatures("modules")>>

new module foo {}
module foo;

internal class Foo {}
// ^ hover-at-caret
