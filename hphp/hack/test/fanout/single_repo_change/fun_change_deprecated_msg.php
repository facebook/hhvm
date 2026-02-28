//// base-foo.php
<?hh
<<__Deprecated("deprecated")>>
function foo(): void {}
//// base-a.php
<?hh
function take_int(int $_): void {}
class A {
  public function bar(): void {
    take_int(foo());
  }
}
//// changed-foo.php
<?hh
<<__Deprecated("this function is deprecated")>>
function foo(): void {}
//// changed-a.php
<?hh
function take_int(int $_): void {}
class A {
  public function bar(): void {
    take_int(foo());
  }
}
