//// modules.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>

module A {}
module B {}

//// A.php
<?hh
<<file:__EnableUnstableFeatures('modules'), __Module("A")>>

<<__Internal>>
function a(): void {}

function a2(): void { a(); /* ok */ }

//// B.php
<?hh

<<file:__EnableUnstableFeatures('modules'), __Module("B")>>

function b(): void { a(); /* error */ }

function b2(): void { a2(); /* ok */ }

//// main.php
<?hh

function main(): void { a(); /* error */ }
