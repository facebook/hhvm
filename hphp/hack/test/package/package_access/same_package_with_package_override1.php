//// pkg6/foo.php
<?hh
// both files override down into pkg1 (valid: pkg6 includes pkg1), so they end
// up in the same package
<<file: __PackageOverride('pkg1')>>
type TFoo = int;

//// pkg6/bar.php
<?hh
<<file: __PackageOverride('pkg1')>>
type TBar = TFoo;
