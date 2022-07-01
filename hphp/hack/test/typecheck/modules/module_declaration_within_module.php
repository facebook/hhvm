<?hh
<<file:__EnableUnstableFeatures("modules")>>
new module foo {} // not ok
module foo;
new module bar {} // not ok
