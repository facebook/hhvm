<?hh

// This file does stuff.

// This function does stuff.
//
// More doc info here.
function foo(): void {}

function call_it(): void {
  foo();
//  ^ hover-at-caret
}
