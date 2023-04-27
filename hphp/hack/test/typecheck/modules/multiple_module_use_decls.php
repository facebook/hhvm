//// modules.php
<?hh


new module A {}
new module B {}

//// A1.php
<?hh

module A;
module B; // TODO, error here

internal function f(): void {}


//// A2.php
<?hh

module A;

function g(): void { f(); }
