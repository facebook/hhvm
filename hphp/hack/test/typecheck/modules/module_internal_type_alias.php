//// module_A.php
<?hh
new module A {}
//// module_B.php
<?hh
new module B {}

//// A.php
<?hh


module A;

internal type AInt = int;
internal type AVecInt = vec<AInt>; // Ok

//// B.php
<?hh


module B;

internal type BVecInt = vec<AInt>; // Error
