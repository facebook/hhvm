<?hh
require_once('connect.inc');
<<__EntryPoint>> function main(): void {
$conn = mysql_connect($host, $user, $passwd);
$dbs = mysql_list_dbs();
var_dump((bool)mysql_db_name($dbs, 0));
}
