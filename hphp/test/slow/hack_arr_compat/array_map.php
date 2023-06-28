<?hh

// Copyright 2004-present Facebook. All Rights Reserved.

<<__EntryPoint>>
function main_array_map() :mixed{
array_map($x ==> $x, darray(dict['x' => 3]));
array_filter(darray(dict['x' => 3]), $x ==> true);
echo "DONE\n";
}
