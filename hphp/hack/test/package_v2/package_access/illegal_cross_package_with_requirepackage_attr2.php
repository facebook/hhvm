//// a.php
<?hh
// package pkg1

class A {
  <<__RequirePackage("pkg2")>>
  public function test() : void {
  }
}
function test(): void {
  $x = new A();
  $x->test(); // error
}
