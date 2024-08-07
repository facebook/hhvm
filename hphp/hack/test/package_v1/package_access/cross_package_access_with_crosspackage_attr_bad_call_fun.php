//// modules.php
<?hh
new module a {}     // package pkg1
//// a.php
<?hh

module a;
class A {
  public function test() : void {
    test(); // error
  }
}
<<__CrossPackage("pkg2")>>
function test(): void {
}
