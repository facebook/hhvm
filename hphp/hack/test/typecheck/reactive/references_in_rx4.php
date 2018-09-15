<?hh

// ERROR
<<__Rx>>
function &foo(): int {
  throw new Exception();
}
