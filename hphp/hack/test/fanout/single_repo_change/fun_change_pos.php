//// base-foo.php
<?hh
function foo(): void {}
//// base-a.php
<?hh
class A {
  public function bar(): void {
    foo();
  }
}
//// changed-foo.php
<?hh
// Position change here
function foo(): void {}
//// changed-a.php
<?hh
class A {
  public function bar(): void {
    foo();
  }
}
