//// illegal_cross_package_basic_foo.php
<?hh
// package pkg2
type TFoo = int;

//// bar.php
<?hh
// package pkg1
type TBar = TFoo;
