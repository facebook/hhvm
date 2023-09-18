//// module_a.php
<?hh
new module a {} // package pkg1
//// module_b.php
<?hh
new module b.b1 {} // package pkg2
//// module_c.php
<?hh
new module c {} // package pkg3

//// b.php
<?hh
module b.b1;
function pkg2_call(): void {}

//// c.php
<?hh
module c;
function pkg3_call(): void {}

//// a.php
<?hh
<<file:__EnableUnstableFeatures('package')>>
module a;

class A {
  <<__CrossPackage("pkg3")>>
  public function foo(): void {
    pkg3_call();
    pkg2_call(); // error: pkg3 includes pkg2, but you need to explicitly include it here
    invariant(package pkg2, "");
    pkg2_call(); // ok
    pkg1_call();
  }
}
<<__CrossPackage("pkg3")>>
function foo(): void {
  pkg3_call();
  pkg2_call(); // error: pkg3 includes pkg2, but you need to explicitly include it here
  invariant(package pkg2, "");
  pkg2_call(); // ok
  pkg1_call();
}


function pkg1_call(): void {}
