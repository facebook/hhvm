//// a.php
<?hh
// package pkg1

class A {
  <<__RequirePackage("pkgNotDefined5")>> // pkgNotDefined doesn't exist
  public function test() : void {
  }
}
