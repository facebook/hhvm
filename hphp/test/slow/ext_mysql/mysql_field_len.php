<?hh
require_once('connect.inc');
<<__EntryPoint>> function main(): void {
$conn = mysql_connect($host, $user, $passwd);
var_dump(create_test_table('field_len'));
var_dump(mysql_query(
  "insert into test_field_len (name) values ('test'),('test2')"));

$res = mysql_query('select * from test_field_len');
var_dump(mysql_field_len($res, 1));
}
