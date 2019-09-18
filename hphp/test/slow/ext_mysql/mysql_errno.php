<?hh
require_once('connect.inc');
<<__EntryPoint>> function main(): void {
$conn = mysql_connect($host, $user, $passwd);
var_dump(@mysql_select_db('nonexistentdb', $con));
var_dump(mysql_errno($conn));
}
