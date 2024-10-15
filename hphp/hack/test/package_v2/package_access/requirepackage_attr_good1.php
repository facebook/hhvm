//// a.php
<?hh
// package pkg1
<<__EntryPoint>>
function f(): void {}

//// c.php
<?hh
// package pkg3 (unrelated to pkg1)
<<file: __EnableUnstableFeatures('require_package')>>

<<__EntryPoint, __RequirePackage("pkg1")>>
function test() : void {
  f();
}
