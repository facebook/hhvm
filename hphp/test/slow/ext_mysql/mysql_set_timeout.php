<?hh
require_once('connect.inc');
<<__EntryPoint>> function main(): void {
var_dump(mysql_set_timeout(10));
$conn = mysql_connect($host, $user, $passwd, true, 0, 20, 50);
var_dump((bool)$conn);
}
