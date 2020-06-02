<?hh

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/
namespace MyInclude;

const MY_MIN = 10;
const MY_MAX = 50;
<<__EntryPoint>>
function entrypoint_limits(): void {

  \error_reporting(-1);

  echo "================= xxx =================\n";

  echo "Inside file >" . __FILE__ . "< at line >" . __LINE__ .
      "< with namespace >" . __NAMESPACE__ . "<\n";
}
