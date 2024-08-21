//// foo.php
<?hh
// package pkg2
<<file: __PackageOverride('pkg2')>>
function foo(): void {}

//// bar.php
<?hh
// package pkg1
function bar(): void { foo (); }
