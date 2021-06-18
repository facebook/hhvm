<?hh
function my_handler($errno, $errmsg) { return true; }

<<__EntryPoint>>
function main_to_string_recoverable_error_1() {
set_error_handler(my_handler<>);
var_dump((string)(new stdClass));
}
