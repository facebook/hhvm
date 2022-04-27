//// modules.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>

new module A {}
new module B {}

//// A1.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>
module A;
module B; // TODO, error here

internal function f(): void {}


//// A2.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>
module A;

function g(): void { f(); }
