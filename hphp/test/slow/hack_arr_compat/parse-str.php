<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

parse_str('123=value&456[]=foo+bar&789[]=baz', $output);
var_dump($output);
