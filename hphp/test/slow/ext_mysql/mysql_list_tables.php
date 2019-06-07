<?hh
require_once('connect.inc');

$conn = mysql_connect($host, $user, $passwd);
$res = mysql_list_tables($db);
$table = mysql_fetch_row($res);
var_dump($table[0] ?? false);
