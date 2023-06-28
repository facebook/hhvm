<?hh


<<__EntryPoint>>
function main_number_format_long_precision() :mixed{
var_dump(number_format(0.0, 128));
var_dump(number_format(0.0, 28));
}
