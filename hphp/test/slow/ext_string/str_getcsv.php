<?hh


<<__EntryPoint>>
function main_str_getcsv() :mixed{
var_dump(str_getcsv("a,b\n"));
var_dump(str_getcsv("a,b\nc,d"));
}
