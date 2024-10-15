//// a.php
<?hh
// package pkg1

<<file: __EnableUnstableFeatures('require_package')>>

class A {
  <<__RequirePackage("pkgNotDefined5")>> // pkgNotDefined doesn't exist
  public function test() : void {
  }
}
