<?hh

function foo(): nonnull {
  //            ^ hover-at-caret
  throw new Exception();
}
