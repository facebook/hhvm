//// module_x.php
<?hh
new module x {}    // default package

//// x.php
<?hh
module x;
type XInt = int;
function foo(): void {}

//// a.php
<?hh
// undefined module, but would belong to default package
module y;

type YInt = XInt; // ok
function test(): void {
  foo(); // ok
}
