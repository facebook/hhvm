<?hh
require_once('connect.inc');

$conn = mysql_connect($host, $user, $passwd);
var_dump(mysql_get_host_info() ?? false);
