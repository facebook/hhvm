<?hh
<<__EntryPoint>> function main(): void {
require_once('connect.inc');
list($host, $user, $passwd, $db) = connection_settings();
$conn = mysql_connect($host, $user, $passwd);
var_dump(create_test_table($db, 'affected_rows'));
var_dump(mysql_query(
  "INSERT INTO test_affected_rows (name) values ('test'),('test2')"));
var_dump(mysql_affected_rows());
}
