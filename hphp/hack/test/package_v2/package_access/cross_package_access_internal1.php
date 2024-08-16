//// modules.php
<?hh
new module foo {}

//// foo.php
<?hh
// package pkg2
module foo;
internal type TFoo = int;

//// bar.php
<?hh
// package pkg1
type TBar = TFoo;
