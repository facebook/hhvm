<?hh
<<__EntryPoint>> function main(): void {
require_once('connect.inc');
list($host, $user, $passwd, $db) = connection_settings();
$conn = mysql_connect($host, $user, $passwd);
$tables = mysql_list_tables($db);
var_dump((bool)mysql_tablename($tables, 0));
}
