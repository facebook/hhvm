<?hh

// This function does stuff.
/* HH_FIXME[4410] Shameful fixme. */
function foo(): void {}

function call_it(): void {
  foo();
//  ^ hover-at-caret
}
