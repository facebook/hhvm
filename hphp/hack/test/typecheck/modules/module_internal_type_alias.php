//// modules.php
<?hh


new module A {}
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
