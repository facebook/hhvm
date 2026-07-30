//// a.php
<?hh
// package pkg1
class A {}

//// pkg2/b.php
<?hh
class B {}

//// pkg3/c.php
<?hh
<<__EntryPoint>>
function test(): void {
   $b = new B(); // ok
   $a = new A(); // ok by transitive inclusion
   $d = new D(); // error
}

//// pkg4/d.php
<?hh
// package pkg4 by path
class D {}
