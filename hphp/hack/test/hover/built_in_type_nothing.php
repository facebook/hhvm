<?hh

function foo(): nothing {
  //            ^ hover-at-caret
  throw new Exception();
}
