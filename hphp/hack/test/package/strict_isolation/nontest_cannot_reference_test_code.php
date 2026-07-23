//// isolated/__tests__/helper.php
<?hh
// Test code (an excluded path) within the strict-isolation package `isolated`.
const int ISOLATED_TEST_HELPER = 1;

//// isolated/a.php
<?hh
// Non-test code cannot reference test code within a strict-isolation package,
// even in the same package.
const int ISOLATED_C = ISOLATED_TEST_HELPER;
