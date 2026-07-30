//// pkg3/a.php
<?hh

type AInt = int;
function f(): void {}

//// c.php
<?hh

<<__RequirePackage("pkg3")>>
function test(AInt $a) : void {
  f();
}
