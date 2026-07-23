//// isolated/a.php
<?hh
// belongs to the strict-isolation package `isolated`
const int ISOLATED_C = 1;

//// isolated/__tests__/t.php
<?hh
// Test code within `isolated` may reference the package's own non-test code:
// same package, and test -> non-test is allowed.
const int T_C = ISOLATED_C;
