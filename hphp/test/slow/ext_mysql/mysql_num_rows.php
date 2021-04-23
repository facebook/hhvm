<?hh
<<__EntryPoint>> function main(): void {
require_once('connect.inc');
list($host, $user, $passwd, $db) = connection_settings();
$conn = mysql_connect($host, $user, $passwd);
var_dump(create_test_table($db, 'num_rows'));
var_dump(mysql_query(
  "insert into test_num_rows (name) values ('test'),('test2')"));

$res = mysql_query('select * from test_num_rows');
var_dump(mysql_num_rows($res));
}
