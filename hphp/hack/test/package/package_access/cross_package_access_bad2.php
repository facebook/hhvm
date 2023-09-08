//// modules.php
<?hh
new module a {}     // package pkg1
new module b.b1 {}  // package pkg2 (include pkg1)
new module c {}     // package pkg3 (include pkg2)

//// a.php
<?hh
module a;
public class A {}

//// b.php
<?hh
module b.b1;
public class B1 {}

//// c.php
<?hh
module c;
public class C {}
public function test(): void {
   $b = new B1(); // ok
   $a = new A(); // error
}
