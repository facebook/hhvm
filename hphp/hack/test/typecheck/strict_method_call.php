<?hh

function any() {}

function test(): void {
  // The method 'bar' does not exist but that's ok
  any()->bar();
}
