<?hh

<<file:__EnableUnstableFeatures('modules')>>

new module A {}
new module B {}

module A;
// This should probably error, and if it doesn't, it should be consistent
// between legacy decl and direct decl
module B;


function h(): void {}
