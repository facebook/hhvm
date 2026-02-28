<?hh

function foo<reify T>(): void {}

function test(): void {
  foo<>;
}
