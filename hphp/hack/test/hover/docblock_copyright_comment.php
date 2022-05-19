<?hh
// ACME Inc.

function foo(): void {}

function call_it(): void {
  // Should not show the file header when hovering here.
  foo();
//  ^ hover-at-caret
}
