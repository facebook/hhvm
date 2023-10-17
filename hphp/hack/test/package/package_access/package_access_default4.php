//// module_a.php
<?hh
new module b.b1 {}    // pkg2

//// x.php
<?hh
module b.b1;
type XInt = int;
function foo(): void {}

//// a.php
<?hh
// undefined module, but would belong to pkg1
module b.undefined;

type YInt = XInt; // error
function test(): void {
  foo(); // error
}
