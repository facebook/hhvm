//// module_A.php
<?hh
new module A {}
//
//// module_B.php
<?hh
new module B {}
//
//// A.php
<?hh
module A;

internal interface F<+T> {}
interface G extends F<string> {}
class C implements F<arraykey> {}

//// B.php
<?hh
module B;
class D extends C implements G {}
