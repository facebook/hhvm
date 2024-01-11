//// module_a.php
<?hh
new module a {}     // package pkg1
//// module_c.php
<?hh
new module c {}     // package pkg3 (include pkg2)
//// module_b.php
<?hh
new module b.b1 {}  // package pkg2
//// module_d.php
<?hh
new module d {}     // package pkg4

//// a.php
<?hh
module a;
public function f1(): void {}

//// d.php
<?hh
module d;
public function f4(): void {}

//// c.php
<?hh

module c;
public class C {
  public function test() : void {
    if (package pkg1 == package pkg4) {
      f1(); // error; comparison binops don't register package info
    }
    if (package pkg1 is bool) {
      f1(); // error; type ops don't register package info
    }
    if ($this->expect(package pkg1)) {
      f1(); // error; function calls don't register package info
    }
    if ($loaded = package pkg1) {
      f1(); // error; assignments aren't allowed in conditionals

      if ($loaded) {
        f1(); // error; cannot infer $loaded to be a package expression
      }
    }
  }

  private function expect(bool $x) : bool {
    return $x;
  }
}
