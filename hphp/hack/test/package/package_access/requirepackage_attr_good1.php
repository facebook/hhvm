//// a.php
<?hh
<<file: __PackageOverride('pkg3')>>

type AInt = int;
function f(): void {}

//// c.php
<?hh

<<__RequirePackage("pkg3")>>
function test(AInt $a) : void {
  f();
}
