<?php
include "connect.inc";

$tmp    = NULL;
$link   = NULL;

if (NULL !== ($tmp = @mysql_db_name()))
	printf("[001] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

if (NULL !== ($tmp = @mysql_db_name($link, $link)))
	printf("[002] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

require('table.inc');

if (!$res = @mysql_list_dbs($link))
	printf("[003] [%d] %s\n", mysql_errno($link), mysql_error($link));

if (!$num = mysql_num_rows($res))
	printf("[004] Empty database list? [%d] %s\n", mysql_errno($link), mysql_error($link));

if (false !== ($tmp = mysql_db_name($res, -1)))
	printf("[005] Expecting boolean/false, got %s/%s. [%d] %s\n",
		gettype($tmp), $tmp, mysql_errno($link), mysql_error($link));

if (false !== ($tmp = mysql_db_name($res, $num + 1)))
	printf("[006] Expecting boolean/false, got %s/%s. [%d] %s\n",
		gettype($tmp), $tmp, mysql_errno($link), mysql_error($link));

$unicode = (boolean)(version_compare(PHP_VERSION, '5.9.9', '>') == 1);
for ($i = 0; $i < $num; $i++) {
	if ('' === ($dbname = mysql_db_name($res, $i)))
		printf("[%03d] Got empty database name! [%d] %s\n",
			(($i * 2) + 1) + 6, mysql_errno($link), mysql_error($link));

	if ($unicode && !is_unicode($dbname)) {
		printf("[%03d] Expecting unicode string! [%d] %s\n",
			(($i * 2) + 2) + 6, mysql_errno($link), mysql_error($link));
		var_inspect($dbname);
	}
}

mysql_free_result($res);

if (false !== ($tmp = mysql_db_name($res, $num)))
	printf("[999] Expecting boolean/false, got %s/%s. [%d] %s\n",
		gettype($tmp), $tmp, mysql_errno($link), mysql_error($link));

mysql_close($link);

print "done!\n";
?>