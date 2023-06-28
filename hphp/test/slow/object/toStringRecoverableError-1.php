<?hh
function my_handler($errno, $errmsg) :mixed{ return true; }

<<__EntryPoint>>
function main_to_string_recoverable_error_1() :mixed{
set_error_handler(my_handler<>);
var_dump((string)(new stdClass));
}
