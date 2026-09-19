//// pkg2/a.php
<?hh
// package pkg2

function test_in_pkg2(): void {
  // Error: pkg1 does not strictly include pkg2 (pkg2 includes pkg1, not vice versa)
  if (package pkg1) {
    // ...
  }

  // OK: pkg3 strictly includes pkg2
  if (package pkg3) {
    // ...
  }
}

//// b.php
<?hh
// package pkg1

function test_in_pkg1(): void {
  // Error: pkg1 equals pkg1 (same package)
  if (package pkg1) {
    // ...
  }

  // OK: pkg2 strictly includes pkg1
  if (package pkg2) {
    // ...
  }

  // OK: pkg3 strictly includes pkg1
  if (package pkg3) {
    // ...
  }
}

//// pkg2/c.php
<?hh
// package pkg2

function test_soft_includes(): void {
  // OK: a soft include no longer bans the check. pkg2_soft strictly includes
  // pkg2, which is all the strict-inclusion rule asks for.
  if (package pkg2_soft) {
    // ...
  }
}
