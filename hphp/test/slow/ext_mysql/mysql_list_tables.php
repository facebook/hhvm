<?hh
require_once('connect.inc');
<<__EntryPoint>> function main(): void {
$conn = mysql_connect($host, $user, $passwd);
$res = mysql_list_tables($db);
$table = mysql_fetch_row($res);
var_dump($table[0] ?? false);
}
