<?hh
require_once('connect.inc');
<<__EntryPoint>> function main(): void {
$conn = mysql_connect($host, $user, $passwd);
$item = "Zak's Laptop";
var_dump(mysql_real_escape_string($item));
mysql_close($conn);
var_dump(@mysql_real_escape_string($item));
}
