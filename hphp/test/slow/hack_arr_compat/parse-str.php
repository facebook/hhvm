<?hh

// Copyright 2004-present Facebook. All Rights Reserved.

<<__EntryPoint>>
function main_parse_str() :mixed{
$output = null;
parse_str('123=value&456[]=foo+bar&789[]=baz', inout $output);
var_dump($output);
}
