<?hh


// disable array -> "Array" conversion notice
<<__EntryPoint>>
function main_527() {
error_reporting(error_reporting() & ~E_NOTICE);

var_dump(array_unique(varray[varray[1,2], varray[1,2], varray[3,4],]));
}
