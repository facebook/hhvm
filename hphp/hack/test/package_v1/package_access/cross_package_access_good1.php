//// module_a.php
<?hh
new module a {}    // package pkg1
//// module_b.php
<?hh
new module b.b1 {} // package pkg2 (include pkg1)

//// a.php
<?hh
module a;
public class A {}

//// b.php
<?hh
module b.b1;
public class B1 {}
public function test(): void {
   $a = new A(); // ok
}
