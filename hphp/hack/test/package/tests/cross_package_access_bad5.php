//// modules.php
<?hh
new module a {}

//// a.php
<?hh
// package pkg1
module a;
public class A {}


//// b.php
<?hh
// default package
class B {
  public function test(): void {
    $a = new A(); // error
  }
}
