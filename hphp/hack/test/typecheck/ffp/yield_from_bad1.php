<?hh // partial

function foo(): void {
  bar(yield from boo());
}
