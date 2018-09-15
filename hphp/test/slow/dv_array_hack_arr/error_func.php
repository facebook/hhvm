<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function withargs(string $abc) : void {
  var_dump(debug_backtrace());
}

function main() {
  withargs("abc");

  return 0;
}
main();
