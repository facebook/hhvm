<?php
require_once('table.inc');

$res1 = mysql_query('SELECT id FROM test', $link);

if (!$res2 = @mysql_db_query($db, 'SELECT id FROM test', $link))
	printf("[001] [%d] %s\n", mysql_errno($link), mysql_error($link));
mysql_free_result($res2);
print @mysql_escape_string("I don't mind character sets, do I?\n");

$res3 = mysql_query('BOGUS_SQL', $link);
mysql_close($link);

print "done!\n";
?>
<?php
require_once("clean_table.inc");
?>