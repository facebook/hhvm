//// modules_x.php
<?hh
new module x {}    // default package
//// module_a.php
<?hh
new module a {}    // pkg1

//// x.php
<?hh
module x;
public type XInt = int;
function foo(): void {}

//// a.php
<?hh
module a;
public type YInt = XInt; // ok
function test(): void {
  foo(); // ok
}
