//// a.php
<?hh
// package pkg1
function test(): void {
   $b = (nameof B); // nameof should not cause a package boundary violation
}

//// pkg2/b.php
<?hh
// package pkg2
class B {}
