//// modules.php
<?hh
new module b {}     // package pkg1
new module c {}     // package pkg3

//// b.php
<?hh
module b;
public class B {}

//// c.php
<?hh
module c;
public class C {}
public function test(): void {
   $b = new B(); // error
}
