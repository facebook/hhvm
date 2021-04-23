<?hh
<<__EntryPoint>> function main(): void {
require_once('connect.inc');
list($host, $user, $passwd, $db) = connection_settings();
$conn = mysql_connect($host, $user, $passwd);
var_dump(create_test_table($db, 'free_result'));

$res = mysql_query('select * from test_free_result');
var_dump(mysql_free_result($res));
}
