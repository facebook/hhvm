<?hh

// Copyright 2004-present Facebook. All Rights Reserved.

<<__EntryPoint>>
function main_array_map() {
array_map($x ==> $x, darray['x' => 3]);
array_filter(darray['x' => 3], $x ==> true);
echo "DONE\n";
}
