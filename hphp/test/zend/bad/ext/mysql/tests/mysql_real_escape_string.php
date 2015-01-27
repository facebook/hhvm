<?php
include "connect.inc";

$tmp    = NULL;
$link   = NULL;

if (NULL !== ($tmp = @mysql_real_escape_string()))
	printf("[001] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

require('table.inc');

if (NULL !== ($tmp = @mysql_real_escape_string('foo', $link, $link)))
	printf("[002] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

var_dump(mysql_real_escape_string("Am I a unicode string in PHP 6?", $link));
var_dump(mysql_real_escape_string('\\', $link));
var_dump(mysql_real_escape_string('"', $link));
var_dump(mysql_real_escape_string("'", $link));
var_dump(mysql_real_escape_string("\n", $link));
var_dump(mysql_real_escape_string("\r", $link));
var_dump($tmp = mysql_real_escape_string("foo" . chr(0) . "bar", $link));

assert($tmp === mysql_real_escape_string("foo" . chr(0) . "bar"));

print "done!";
?>
