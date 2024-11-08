//// modules.php
<?hh
new module bar {}

//// illegal_cross_package_basic_foo.php
<?hh
// package pkg2
type TFoo = int;

//// bar.php
<?hh
// package pkg1
module bar;

type TBar = TFoo; // error
newtype NBar1 = TFoo; // ok
newtype NBar2 as TFoo = int; // ok
module newtype NBar3 = TFoo; // ok
module newtype NBar4 as TFoo = int; // ok
