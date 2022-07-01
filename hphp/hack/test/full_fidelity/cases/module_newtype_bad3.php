<?hh
<<file:__EnableUnstableFeatures("modules")>>
module foo;
async newtype Foo = int; // error
