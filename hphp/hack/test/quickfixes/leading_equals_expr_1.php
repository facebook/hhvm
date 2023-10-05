<?hh

function bar(mixed $_): void {}

function foo(): void {
  $x = 1;
  // This refactor is probably not very helpful
  // but at least produces valid Hack
  bar(=$x);
}
