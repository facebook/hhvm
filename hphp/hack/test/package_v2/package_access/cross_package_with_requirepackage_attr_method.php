//// b.php
<?hh
<<file: __PackageOverride('pkg2')>>
function pkg2_call(): void {}

//// c.php
<?hh
<<file: __PackageOverride('pkg3')>>
function pkg3_call(): void {}

//// a.php
<?hh
// package pkg1
<<file: __EnableUnstableFeatures('require_package')>>
class A {
  <<__RequirePackage("pkg3")>>
  public function foo(): void {
    pkg3_call();
    pkg2_call(); // error: pkg3 includes pkg2, but you need to explicitly include it here
    invariant(package pkg2, "");
    pkg2_call(); // ok
    pkg1_call();
  }
}
<<__RequirePackage("pkg3")>>
function foo(): void {
  pkg3_call();
  pkg2_call(); // error: pkg3 includes pkg2, but you need to explicitly include it here
  invariant(package pkg2, "");
  pkg2_call(); // ok
  pkg1_call();
}


function pkg1_call(): void {}
