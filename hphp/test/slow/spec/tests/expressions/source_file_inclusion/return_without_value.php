<?hh
<<__EntryPoint>>
function entrypoint_return_without_value(): void {

  /*
     +-------------------------------------------------------------+
     | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
     +-------------------------------------------------------------+
  */

  error_reporting(-1);

  // return without a value, so when included, NULL is returned

  return;
}
