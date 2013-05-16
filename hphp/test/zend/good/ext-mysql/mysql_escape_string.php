<?php
include "connect.inc";

$tmp    = NULL;
$link   = NULL;

if (NULL !== ($tmp = @mysql_escape_string()))
	printf("[001] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

var_dump(@mysql_escape_string("Am I a unicode string in PHP 6?"));
var_dump(@mysql_escape_string('\\'));
var_dump(@mysql_escape_string('"'));
var_dump(@mysql_escape_string("'"));
var_dump(@mysql_escape_string("\n"));
var_dump(@mysql_escape_string("\r"));
var_dump(@mysql_escape_string("foo" . chr(0) . "bar"));

print "done!";
?>