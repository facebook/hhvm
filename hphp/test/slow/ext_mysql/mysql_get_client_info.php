<?hh


<<__EntryPoint>>
function main_mysql_get_client_info() :mixed{
var_dump((bool)(mysql_get_client_info() ?? false));
}
