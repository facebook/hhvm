//// base-foo.php
<?hh
function foo(): string { return "s"; }
//// base-a.php
<?hh
function take_string(string $_): void {}
class A {
  public function bar(): void {
    take_string(foo());
  }
}
//// changed-foo.php
<?hh
function foo(): int { return 4; }
//// changed-a.php
<?hh
function take_string(string $_): void {}
class A {
  public function bar(): void {
    take_string(foo());
  }
}
