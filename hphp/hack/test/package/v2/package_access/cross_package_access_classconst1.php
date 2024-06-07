//// a.php
<?hh
// package pkg1
function test(): void {
   $b = B1::class; // ::class should not cause a package boundary violation
}

//// b.php
<?hh
// package pkg2
<<file: __EnableUnstableFeatures('package_v2'), __PackageOverride('pkg2')>>
class B1 {}
