//// moduleA.php
<?hh
new module A {}

//// moduleB.php
<?hh
new module B {}

//// A.php
<?hh

module A;

internal function a(): void {}

function a2(): void { a(); /* ok */ }

//// B.php
<?hh


module B;

function b(): void { a(); /* error */ }

function b2(): void { a2(); /* ok */ }

//// main.php
<?hh

function main(): void { a(); /* error */ }
