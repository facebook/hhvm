<?php
require_once("connect.inc");

$tmp    = NULL;
$link   = NULL;

// string mysql_result ( resource result, int row [, mixed field] )

if (!is_null($tmp = @mysql_result()))
	printf("[001] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

if (!is_null($tmp = @mysql_result($link)))
	printf("[002] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

require_once('table.inc');
if (!$res = mysql_query("SELECT id, label, id AS _id, CONCAT(label, 'a') _label, NULL as _foo FROM test _test ORDER BY id ASC LIMIT 1", $link)) {
	printf("[003] [%d] %s\n", mysql_errno($link), mysql_error($link));
}

var_dump(mysql_result($res, -1));
var_dump(mysql_result($res, 2));

var_dump(mysql_result($res, 0, -1));
var_dump(mysql_result($res, 0, 500));

print "valid fields\n";
var_dump(mysql_result($res, 0));
var_dump(mysql_result($res, 0, 1));

var_dump(mysql_result($res, 0, 'id'));
var_dump(mysql_result($res, 0, '_test.id'));
var_dump(mysql_result($res, 0, 'label'));
var_dump(mysql_result($res, 0, '_test.label'));
print "some invalid fields\n";
var_dump(mysql_result($res, 0, 'unknown'));
var_dump(mysql_result($res, 0, '_test.'));
var_dump(mysql_result($res, 0, chr(0)));
var_dump(mysql_result($res, 0, '_test.' . chr(0)));
print "_id\n";
var_dump(mysql_result($res, 0, '_id'));
print "_label\n";
var_dump(mysql_result($res, 0, '_label'));
print "_foo\n";
var_dump(mysql_result($res, 0, '_foo'));
var_dump(mysql_result($res, 0, 'test.id'));
var_dump(mysql_result($res, 0, 'test.label'));

mysql_free_result($res);

var_dump(mysql_result($res, 0));

mysql_close($link);
print "done!";
?>
<?php
require_once("clean_table.inc");
?>