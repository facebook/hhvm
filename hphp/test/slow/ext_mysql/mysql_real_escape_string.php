<?hh
<<__EntryPoint>> function main(): void {
require_once('connect.inc');
list($host, $user, $passwd, $db) = connection_settings();
$conn = mysql_connect($host, $user, $passwd);
$item = "Zak's Laptop";
var_dump(mysql_real_escape_string($item));
mysql_close($conn);
var_dump(@mysql_real_escape_string($item));
}
