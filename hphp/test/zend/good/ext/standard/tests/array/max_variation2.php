<?hh
/*
 * proto mixed max(mixed arg1 [, mixed arg2 [, mixed ...]])
 * Function is implemented in ext/standard/array.c
*/
<<__EntryPoint>> function main(): void {
echo "\n*** Testing arrays  ***\n";

var_dump(max(varray[2,1,2]));
var_dump(max(varray[-2,1,2]));
var_dump(max(varray[2.1,2.11,2.09]));
var_dump(max(varray["", "t", "b"]));
var_dump(max(varray[false, true, false]));
var_dump(max(varray[true, false, true]));
var_dump(max(varray[1, true, false, true]));
var_dump(max(varray[0, true, false, true]));
var_dump(max(varray[0, 1, NULL]));
var_dump(max(varray[2147483645, 2147483646]));
var_dump(max(varray[2147483647, 2147483648]));
var_dump(max(varray[2147483646, 2147483648]));
var_dump(max(varray[-2147483647, -2147483646]));
var_dump(max(varray[-2147483648, -2147483647]));
var_dump(max(varray[-2147483649, -2147483647]));

echo "\nDone\n";
}
