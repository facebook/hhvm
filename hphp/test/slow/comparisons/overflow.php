<?hh


<<__EntryPoint>>
function main_overflow() :mixed{
var_dump("1"<"10000000000000000000.0");
var_dump("10000000000000000000.0" > "1");
}
