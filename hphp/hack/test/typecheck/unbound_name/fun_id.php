<?hh

function foo(): void {}

function test(): void {
  foo<>;

  // Should error
  not_found<>;
}
