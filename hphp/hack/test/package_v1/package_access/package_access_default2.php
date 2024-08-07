//// module_x.php
<?hh
new module x {}    // default package
//// modules_a.php
<?hh
new module a {}    // pkg1

//// a.php
<?hh
module a;
public type XInt = int;
function foo(): void {}

//// x.php
<?hh
<<file:__EnableUnstableFeatures("package")>>
module x;
public type YInt = XInt; // error
function test(): void {
  if(package pkg1) {
    foo(); // ok
  }
  foo(); // error
}
