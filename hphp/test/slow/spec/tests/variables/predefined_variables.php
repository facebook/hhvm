<?hh

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

<<__EntryPoint>>
function entrypoint_predefined_variables(): void {
  error_reporting(-1);

  var_dump(\HH\global_get('argc'));
  var_dump(\HH\global_get('argv'));
  var_dump(\HH\global_get('_ENV'));
}
