//// modules.php
<?hh
new module x {}    // default package
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
