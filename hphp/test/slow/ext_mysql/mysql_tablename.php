<?hh
require_once('connect.inc');
<<__EntryPoint>> function main(): void {
$conn = mysql_connect($host, $user, $passwd);
$tables = mysql_list_tables($db);
var_dump((bool)mysql_tablename($tables, 0));
}
