//// isolated/a.php
<?hh
// belongs to the strict-isolation package `isolated`
const int ISOLATED_C = 1;

//// intern/__tests__/t.php
<?hh
// A test file (an excluded path) outside `isolated`. For strict-isolation
// packages package_exclude_patterns does not grant a typecheck exemption, so this
// test file cannot reference the package's symbols: cross-package violation.
const int T_C = ISOLATED_C;
