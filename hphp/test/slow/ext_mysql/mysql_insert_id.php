<?hh
<<__EntryPoint>> function main(): void {
require_once('connect.inc');
list($host, $user, $passwd, $db) = connection_settings();
$conn = mysql_connect($host, $user, $passwd);
var_dump(create_test_table($db, 'insert_id'));
var_dump(mysql_query(
  "insert into test_insert_id (name) values ('test'),('test2')"));
var_dump(mysql_insert_id());
}
