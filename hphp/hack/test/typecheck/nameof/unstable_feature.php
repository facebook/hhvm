<?hh

class C {}
function expect_string(string $s): void {}
function expect_classname(classname<C> $s): void {}

class Test {
  public function f(): void {
    expect_string(nameof C);
    expect_classname(nameof static);
    nameof $x;
  }
}
