//// a.php
<?hh
// package pkg1
class A {
  public function test() : void {
    test(); // error
  }
}
<<__CrossPackage("pkg2")>>
function test(): void {}
