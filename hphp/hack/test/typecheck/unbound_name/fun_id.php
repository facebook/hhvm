<?hh

function foo(): void {}

function test(): void {
  fun('foo');

  // Should error
  fun('not_found');
}
