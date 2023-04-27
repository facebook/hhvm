<?hh

/**
 * This function does stuff.
 */

function foo(): void {}

function call_it(): void {
  // Should show the delimiter comment despite the gap.
  foo();
//  ^ hover-at-caret
}
