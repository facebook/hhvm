<?php
include_once "connect.inc";

$tmp    = NULL;
$link   = NULL;

if (!is_null($tmp = @mysql_thread_id($link)))
	printf("[001] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

require('table.inc');

if (!is_int($id = mysql_thread_id($link)) || (0 === $id))
	printf("[002] Expecting int/any but zero, got %s/%s. [%d] %s\n",
		gettype($id), $id, mysql_errno($link), mysql_error($link));

if (!is_int($id_def = mysql_thread_id()) || (0 === $id_def))
	printf("[003] Expecting int/any but zero, got %s/%s. [%d] %s\n",
		gettype($id_def), $id_def, mysql_errno(), mysql_error());

assert($id === $id_def);

mysql_close($link);

if (false !== ($tmp = mysql_thread_id($link)))
	printf("[003] Expecting boolean/false, got %s/%s\n", gettype($tmp), $tmp);

print "done!";
?>
