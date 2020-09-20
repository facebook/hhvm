<?hh
require_once('connect.inc');
<<__EntryPoint>> function main(): void {
$conn = mysql_connect($host, $user, $passwd);
var_dump(create_test_table('info'));
var_dump(mysql_query("insert into test_info (name) values ('test'),('test2')"));
var_dump(mysql_info());
}
