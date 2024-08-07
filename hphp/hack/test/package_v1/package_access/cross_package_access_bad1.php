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
public function test(): void {
   $b = new B1(); // error
}

//// b.php
<?hh
module b.b1;
public class B1 {}
