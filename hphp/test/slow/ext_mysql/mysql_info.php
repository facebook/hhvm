<?hh
<<__EntryPoint>> function main(): void {
require_once('connect.inc');
list($host, $user, $passwd, $db) = connection_settings();
$conn = mysql_connect($host, $user, $passwd);
var_dump(create_test_table($db, 'info'));
var_dump(mysql_query("insert into test_info (name) values ('test'),('test2')"));
var_dump(mysql_info());
}
