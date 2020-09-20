<?hh


<<__EntryPoint>>
function main_mysql_get_client_info() {
var_dump((bool)(mysql_get_client_info() ?? false));
}
