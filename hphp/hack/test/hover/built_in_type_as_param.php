<?hh

function foo(): vec<arraykey> {
  //                ^ hover-at-caret
  throw new Exception();
}
