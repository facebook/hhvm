<?hh
<<__EntryPoint>>
function entrypoint_return_with_value(): void {

  /*
     +-------------------------------------------------------------+
     | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
     +-------------------------------------------------------------+
  */

  error_reporting(-1);

  // has explicit return value, so when included, that value is returned

  return 987;
}
