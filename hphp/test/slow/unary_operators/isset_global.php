<?hh


<<__EntryPoint>>
function main_isset_global() :mixed{
var_dump(isset($i_dont_exist));

$i_am_null = null;
var_dump(isset($i_am_null));
}
