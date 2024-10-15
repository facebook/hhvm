//// a.php
<?hh
// package pkg2
<<file: __PackageOverride('pkg2')>>
<<file: __EnableUnstableFeatures('require_package')>>

class A {
  public function test() : void {
    test_pkg1(); // ok since pkg2 includes pkg1
    if(package pkg3) {
      test_pkg3(); // ok
    }
  }
}
<<__RequirePackage("pkg1")>>
function test_pkg1(): void {
}
<<__RequirePackage("pkg3")>>
function test_pkg3(): void {
}
