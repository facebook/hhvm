//// pkg6/foo.php
<?hh
<<file: __PackageOverride('pkg1')>>
type TFoo = int;

//// pkg4/bar.php
<?hh
type TBar = TFoo;
