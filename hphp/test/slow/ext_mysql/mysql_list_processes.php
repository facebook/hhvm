<?hh
<<__EntryPoint>> function main(): void {
require_once('connect.inc');
list($host, $user, $passwd, $db) = connection_settings();
$conn = mysql_connect($host, $user, $passwd);
$res = mysql_list_processes();
$process = mysql_fetch_assoc($res);
var_dump($process['Id'] ?? false);
}
