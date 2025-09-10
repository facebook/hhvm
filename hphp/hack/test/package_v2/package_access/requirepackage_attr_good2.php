//// a.php
<?hh
// package pkg1
type AInt = int;

//// c.php
<?hh
// package pkg3 (unrelated to pkg1)

<<__RequirePackage("pkg1")>>
function test(AInt $a) : void {}
