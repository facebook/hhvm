//// a.php
<?hh
// package pkg1
class A {
  <<__CrossPackage("pkg5")>> // pkg5 doesn't exist
  public function test() : void {
  }
}
