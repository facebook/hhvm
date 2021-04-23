<?hh
<<__EntryPoint>> function main(): void {
require_once('connect.inc');
list($host, $user, $passwd, $db) = connection_settings();
$conn = mysql_connect($host, $user, $passwd);
$dbs = mysql_list_dbs();
var_dump((bool)mysql_db_name($dbs, 0));
}
