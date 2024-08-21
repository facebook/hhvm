//// a.php
<?hh
// package pkg1
type AInt = int;

//// c.php
<?hh
// package pkg3 (unrelated to pkg1)
<<__CrossPackage("pkg1")>>
function test(AInt $a) : void {}
