<?php
include "connect.inc";

$tmp    = NULL;
$link   = NULL;

if (!is_null($tmp = @mysql_field_seek()))
	printf("[001] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

if (!is_null($tmp = @mysql_field_seek($link)))
	printf("[002] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

require('table.inc');
if (!$res = mysql_query("SELECT id, label FROM test ORDER BY id LIMIT 1", $link)) {
	printf("[003] [%d] %s\n", mysql_errno($link), mysql_error($link));
}

var_dump(mysql_field_seek($res, -1));
var_dump(mysql_fetch_field($res));
var_dump(mysql_field_seek($res, 0));
var_dump(mysql_fetch_field($res));
var_dump(mysql_field_seek($res, 1));
var_dump(mysql_fetch_field($res));
var_dump(mysql_field_seek($res, 2));
var_dump(mysql_fetch_field($res));

mysql_free_result($res);

var_dump(mysql_field_seek($res, 0));

mysql_close($link);
print "done!";
?>
<?php
require_once("clean_table.inc");
?>