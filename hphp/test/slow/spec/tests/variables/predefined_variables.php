<?hh

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

<<__EntryPoint>>
function entrypoint_predefined_variables(): void {
  error_reporting(-1);

  var_dump($GLOBALS['argc']);
  var_dump($GLOBALS['argv']);
  var_dump($GLOBALS['_ENV']);
  var_dump($GLOBALS['GLOBALS']);
}
