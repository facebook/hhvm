<?hh
/*
 * proto mixed min(mixed arg1 [, mixed arg2 [, mixed ...]])
 * Function is implemented in ext/standard/array.c
*/
<<__EntryPoint>> function main(): void {
echo "\n*** Testing arrays  ***\n";

var_dump(min(varray[2,1,2]));
var_dump(min(varray[-2,1,2]));
var_dump(min(varray[2.1,2.11,2.09]));
var_dump(min(varray["", "t", "b"]));
var_dump(min(varray[false, true, false]));
var_dump(min(varray[true, false, true]));
var_dump(min(varray[1, true, false, true]));
var_dump(min(varray[0, true, false, true]));
var_dump(min(varray[0, 1, NULL]));
var_dump(min(varray[2147483645, 2147483646]));
var_dump(min(varray[2147483647, 2147483648]));
var_dump(min(varray[2147483646, 2147483648]));
var_dump(min(varray[-2147483647, -2147483646]));
var_dump(min(varray[-2147483648, -2147483647]));
var_dump(min(varray[-2147483649, -2147483647]));

echo "\nDone\n";
}
