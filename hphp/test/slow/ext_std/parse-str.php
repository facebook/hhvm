<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test() {
  parse_str("a=1&b=3&c=10", &$output);
  var_dump($output);
}
test();
