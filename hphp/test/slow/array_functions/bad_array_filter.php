<?hh


<<__EntryPoint>>
function main_bad_array_filter() :mixed{
error_reporting(-1);
var_dump(array_filter(varray[1,2,3], 'fizzle'));
}
