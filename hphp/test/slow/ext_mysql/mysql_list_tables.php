<?hh
<<__EntryPoint>> function main(): void {
require_once('connect.inc');
list($host, $user, $passwd, $db) = connection_settings();
$conn = mysql_connect($host, $user, $passwd);
$res = mysql_list_tables($db);
$table = mysql_fetch_row($res);
var_dump($table[0] ?? false);
}
