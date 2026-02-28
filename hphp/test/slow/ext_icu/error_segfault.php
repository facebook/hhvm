<?hh

<<__EntryPoint>>
function main_error_segfault() :mixed{
var_dump(Transliterator::create(str_repeat("x", 20000)));
var_dump(intl_get_error_message());
}
