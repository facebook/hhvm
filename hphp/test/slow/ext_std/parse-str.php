<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test() :mixed{
  $output = null;
  parse_str("a=1&b=3&c=10", inout $output);
  var_dump($output);
}

<<__EntryPoint>>
function main_parse_str() :mixed{
test();
}
