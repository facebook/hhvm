<?hh

<<file:__EnableUnstableFeatures('modules')>>

new module A {}
new module B {}

<<file:__Module('A')>>
// This should probably error, and if it doesn't, it should be consistent
// between legacy decl and direct decl
<<file:__Module('B')>>


function h(): void {}
