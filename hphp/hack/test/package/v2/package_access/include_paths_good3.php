//// foo.php
<?hh
// package pkg2
<<file: __EnableUnstableFeatures('package_v2'), __PackageOverride('pkg2')>>
function foo(): void {}

//// bar.php
<?hh
// package pkg2
<<file: __EnableUnstableFeatures('package_v2'), __PackageOverride('pkg2')>>
function bar(): void { foo (); }
