<?hh
require_once('connect.inc');
<<__EntryPoint>> function main(): void {
$conn = mysql_connect($host, $user, $passwd);
$res = mysql_list_processes();
$process = mysql_fetch_assoc($res);
var_dump($process['Id'] ?? false);
}
