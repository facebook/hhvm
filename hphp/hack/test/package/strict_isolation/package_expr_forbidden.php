//// intern/i.php
<?hh
// The `package` expression cannot be used to dynamically observe a
// strict-isolation package.
function test_package_expr(): void {
  if (package isolated) {
  }
}
