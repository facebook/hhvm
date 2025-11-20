//// foo.php
<?hh
// package pkg1
type TFoo = int;

const int FOO = 3;

//// bar.php
<?hh
// package pkg1
type TBar = TFoo;

const int BAR = FOO;
