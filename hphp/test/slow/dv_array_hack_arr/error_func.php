<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function withargs(string $abc) : void {
  var_dump(debug_backtrace());
}

function main() :mixed{
  withargs("abc");

  return 0;
}
<<__EntryPoint>>
function entrypoint_error_func(): void {
  main();
}
