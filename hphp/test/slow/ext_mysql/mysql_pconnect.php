<?hh
require_once('connect.inc');
<<__EntryPoint>> function main(): void {
$conn = mysql_pconnect($host, $user, $passwd);
var_dump((bool)$conn);
}
