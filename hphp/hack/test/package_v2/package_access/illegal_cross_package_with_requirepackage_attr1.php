//// a.php
<?hh
// package pkg1

<<file:__EnableUnstableFeatures('require_package')>>

class A {
  public function test() : void {
    test(); // error
  }
}
<<__RequirePackage("pkg2")>>
function test(): void {}
