//// a.php
<?hh
// package pkg1
function test(): void {
   $b = B1::class; // error when package_allow_classconst_violations is off
}

//// b.php
<?hh
// package pkg2
<<file: __PackageOverride('pkg2')>>
class B1 {}
