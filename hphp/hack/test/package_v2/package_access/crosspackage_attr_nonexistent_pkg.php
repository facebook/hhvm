//// a.php
<?hh
// package pkg1
class A {
  <<__CrossPackage("pkgNotDefined5")>> // pkgNotDefined doesn't exist
  public function test() : void {
  }
}
