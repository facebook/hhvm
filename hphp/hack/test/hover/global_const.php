<?hh

/** A doc comment. */
const int BAR = 1;

function use_it(): void {
  $x = BAR;
  //   ^ hover-at-caret
}
