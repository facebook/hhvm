//// a.php
<?hh
// package pkg1
type AInt = int;

//// c.php
<?hh
// package pkg3 (unrelated to pkg1)
<<file: __EnableUnstableFeatures('require_package')>>

<<__RequirePackage("pkg1")>>
function test(AInt $a) : void {}
