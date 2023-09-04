//// base-foo.php
<?hh
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
// Position change here, check in the logs whether the error
// position is properly updated.
//
// Note that A will not necessarily be in the fanout, in hh_server
// we automatically re-typecheck all files with errors.
function foo(): void {}
//// changed-a.php
<?hh
function take_int(int $_): void {}
class A {
  public function bar(): void {
    take_int(foo());
  }
}
