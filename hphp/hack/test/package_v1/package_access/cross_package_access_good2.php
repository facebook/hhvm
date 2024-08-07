//// module_a.php
<?hh
new module a {}    // package pkg1
//// module_b.php
<?hh
new module b {}    // package pkg1

//// a.php
<?hh
module a;
public class A {}

//// b.php
<?hh
module b;
public class B {}
public function test(): void {
   $a = new A(); // ok
}
