<?hh
require_once('connect.inc');
<<__EntryPoint>> function main(): void {
$conn = mysql_connect($host, $user, $passwd);
var_dump(create_test_table('fetch_field'));
var_dump(mysql_query(
  "insert into test_fetch_field (name) values ('test'),('test2')"));

$res = mysql_query('select * from test_fetch_field');
var_dump(mysql_fetch_field($res, 1)->name);
}
