//// modules.php
<?hh
new module b.b1 {}     // package pkg2
//// a.php
<?hh

module b.b1;
class A {
  public function test() : void {
    test_pkg1(); // ok since pkg2 includes pkg1
    if(package pkg3) {
      test_pkg3(); // ok
    }
  }
}
<<__CrossPackage("pkg1")>>
function test_pkg1(): void {
}
<<__CrossPackage("pkg3")>>
function test_pkg3(): void {
}
