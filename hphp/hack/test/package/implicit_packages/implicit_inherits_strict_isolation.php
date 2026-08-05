//// intern/i.php
<?hh
// Implicit packages inherit strict isolation (enable_strict_isolation = true),
// so their presence cannot be dynamically observed: the `package` expression is
// rejected for an implicit family, reported as a strict-isolation error.
function test_package_expr(): void {
  if (package prototypes) {
  }
}
