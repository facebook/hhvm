<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

// ERROR: __MaybeMutable is not allowed on functions
<<__MaybeMutable>>
function f(): int {
  return 1;
}
