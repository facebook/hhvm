<?hh // strict

// ERROR: __MaybeMutable is not allowed on functions
<<__MaybeMutable>>
function f(): int {
  return 1;
}
