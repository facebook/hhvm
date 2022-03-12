<?hh

function foo(): ?int {
  //            ^ hover-at-caret
  throw new Exception();
}
