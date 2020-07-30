<?hh

function foo<reify T>(T $x): void {}

function test(): void {
  $x = foo<int>;

  $x("Hello");
}
