<?hh
<<file:__EnableUnstableFeatures("modules")>>
module foo;
module newtype Foo as int = int;

<<Foo>>
module newtype Bar as int = int;
