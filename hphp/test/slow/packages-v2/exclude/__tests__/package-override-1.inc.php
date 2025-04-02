<?hh

<<file:__PackageOverride("foo")>>

function access_test_package_override(): void {
  echo "I'm ".__FILE__."\n";
}
