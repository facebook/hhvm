<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

namespace SomeNS;

function compact() {
  echo "Fake compact called....\n";
  return [];
}

function main() {
  $var = 100;
  var_dump(compact('var'));
  var_dump(\compact('var'));
  var_dump($var);
  $arr = ['var' => '200'];
  extract(&$arr);
  var_dump($var);
  $arr = ['var' => '300'];
  \extract(&$arr);
  var_dump($var);
}

<<__EntryPoint>>
function main_builtin_shadow() {
main();
}
