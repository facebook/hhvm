<?hh

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

namespace MyColors;

const RED = 1;
const WHITE = 2;
const BLUE = 3;
<<__EntryPoint>>
function entrypoint_mycolors(): void {
  \error_reporting(-1);

  echo "Inside file >" . __FILE__ . "< at line >" . __LINE__ .
      "< with namespace >" . __NAMESPACE__ . "<\n";
}
