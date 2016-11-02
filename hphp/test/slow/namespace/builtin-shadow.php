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
  extract(['var' => '200']);
  var_dump($var);
  \extract(['var' => '300']);
  var_dump($var);
}
main();
