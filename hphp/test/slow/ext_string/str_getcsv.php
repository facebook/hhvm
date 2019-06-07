<?hh


<<__EntryPoint>>
function main_str_getcsv() {
var_dump(str_getcsv("a,b\n"));
var_dump(str_getcsv("a,b\nc,d"));
}
