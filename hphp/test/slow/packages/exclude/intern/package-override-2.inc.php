<?hh

<<file:__PackageOverride("foo")>>

<<__DynamicallyCallable>>
function access_test_package_override_2(): void {
  echo "I'm access_test_package_override_2 in ".__FILE__."\n";
}
