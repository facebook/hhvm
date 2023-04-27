<?hh

function foo(): int {
  /* HH_FIXME[4110] */ return "not a string";
  //  ^ hover-at-caret
}
