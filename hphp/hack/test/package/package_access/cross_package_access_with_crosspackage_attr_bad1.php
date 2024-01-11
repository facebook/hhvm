//// modules.php
<?hh
new module a {}     // package pkg1

//// a.php
<?hh

module a;
class A {
  <<__CrossPackage("pkg5")>>
  public function test() : void {
  }
}
