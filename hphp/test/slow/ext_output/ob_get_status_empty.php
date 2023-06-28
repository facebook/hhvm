<?hh

<<__EntryPoint>>
function main_ob_get_status_empty() :mixed{
ob_start();
var_dump((bool)ob_get_status(false));
var_dump(count(ob_get_status(true)));
var_dump(is_varray(ob_get_status(true)));
var_dump(is_darray(ob_get_status(false)));
}
