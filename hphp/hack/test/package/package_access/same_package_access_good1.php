//// module_a.php
<?hh
new module a {}    // package pkg1
//// module_b.php
<?hh
new module b.b2 {} // package pkg1

//// a.php
<?hh
module a;
public class A {}
public function test1(): void {
   $b = new B2(); // ok
}

//// b.php
<?hh
module b.b2;
public class B2 {}
public function test2(): void {
   $a = new A(); // ok
}
