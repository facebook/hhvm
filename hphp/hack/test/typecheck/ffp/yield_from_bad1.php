<?hh

function foo(): void {
  bar(yield from boo());
}
