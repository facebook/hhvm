<?hh
<<__EntryPoint>> function main(): void {
require_once('connect.inc');
list($host, $user, $passwd, $db) = connection_settings();
$conn = mysql_connect($host, $user, $passwd);
var_dump(mysql_select_db('nonexistentdb', $conn));
var_dump(mysql_errno($conn));
}
