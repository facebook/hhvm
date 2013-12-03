<?php
include "connect.inc";

$tmp    = NULL;
$link   = NULL;

if (NULL !== ($tmp = @mysql_data_seek()))
	printf("[001] Expecting boolean/false, got %s/%s\n", gettype($tmp), $tmp);

if (NULL !== ($tmp = @mysql_data_seek($link)))
	printf("[002] Expecting boolean/false, got %s/%s\n", gettype($tmp), $tmp);

if (NULL !== ($tmp = @mysql_data_seek($link, $link)))
	printf("[003] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

require('table.inc');
if (!$res = mysql_query('SELECT * FROM test ORDER BY id LIMIT 4', $link))
	printf("[004] [%d] %s\n", mysql_errno($link), mysql_error($link));

if (true !== ($tmp = mysql_data_seek($res, 3)))
	printf("[005] Expecting boolean/true, got %s/%s\n", gettype($tmp), $tmp);

$row = mysql_fetch_assoc($res);
if (4 != $row['id'])
	printf("[006] Expecting record 4/d, got record %s/%s\n", $row['id'], $row['label']);

if (true !== ($tmp = mysql_data_seek($res, 0)))
	printf("[007] Expecting boolean/true, got %s/%s\n", gettype($tmp), $tmp);

$row = mysql_fetch_assoc($res);
if (1 != $row['id'])
	printf("[008] Expecting record 1/a, got record %s/%s\n", $row['id'], $row['label']);

if (false !== ($tmp = mysql_data_seek($res, 4)))
	printf("[009] Expecting boolean/false, got %s/%s\n", gettype($tmp), $tmp);

if (false !== ($tmp = mysql_data_seek($res, -1)))
	printf("[010] Expecting boolean/false, got %s/%s\n", gettype($tmp), $tmp);

mysql_free_result($res);

if (!$res = mysql_unbuffered_query('SELECT * FROM test ORDER BY id', $link))
	printf("[011] [%d] %s\n", mysql_errno($link), mysql_error($link));

if (false !== ($tmp = mysql_data_seek($res, 3)))
	printf("[012] Expecting boolean/false, got %s/%s\n", gettype($tmp), $tmp);

mysql_free_result($res);

if (false !== ($tmp = mysql_data_seek($res, 1)))
	printf("[013] Expecting boolean/false, got %s/%s\n", gettype($tmp), $tmp);

mysql_close($link);

print "done!\n";
?>
<?php
require_once("clean_table.inc");
?>