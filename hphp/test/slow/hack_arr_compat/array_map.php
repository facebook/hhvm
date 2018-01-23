<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

array_map($x ==> $x, ['x' => 3]);
array_filter(['x' => 3], $x ==> true);
echo "DONE\n";
