//// foo.php
<?hh
// package pkg2
<<file: __EnableUnstableFeatures('package_v2'), __PackageOverride('pkg2')>>
type TFoo = int;

//// bar.php
<?hh
// package pkg2
<<file: __EnableUnstableFeatures('package_v2'), __PackageOverride('pkg2')>>
type TBar = TFoo;
