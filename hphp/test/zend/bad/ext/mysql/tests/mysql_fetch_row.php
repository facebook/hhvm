<?php
include "connect.inc";

$tmp    = NULL;
$link   = NULL;

if (!is_null($tmp = @mysql_fetch_row()))
	printf("[001] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

if (NULL !== ($tmp = @mysql_fetch_row($link)))
	printf("[002] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

require('table.inc');
if (!$res = mysql_query("SELECT id, label FROM test ORDER BY id LIMIT 1", $link)) {
	printf("[003] [%d] %s\n", mysql_errno($link), mysql_error($link));
}

print "[004]\n";
var_dump(mysql_fetch_row($res));

print "[005]\n";
var_dump(mysql_fetch_row($res));

mysql_free_result($res);

var_dump(mysql_fetch_row($res));

mysql_close($link);
print "done!";
?>
<?php error_reporting(0); ?>
<?php
require_once("clean_table.inc");
?>