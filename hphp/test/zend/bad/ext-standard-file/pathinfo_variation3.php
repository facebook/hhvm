<?php
/* Prototype  : array pathinfo(string path[, int options])
 * Description: Returns information about a certain string 
 * Source code: ext/standard/string.c
 * Alias to functions: 
 */

echo "*** Testing pathinfo() : usage variation ***\n";

$testfile = "/usr/include/arpa/inet.h";

var_dump(pathinfo("./"));
var_dump(pathinfo("/."));
var_dump(pathinfo(".cvsignore"));
var_dump(pathinfo($testfile, PATHINFO_BASENAME));
var_dump(pathinfo($testfile, PATHINFO_FILENAME));
var_dump(pathinfo($testfile, PATHINFO_EXTENSION));
var_dump(pathinfo($testfile, PATHINFO_DIRNAME));
var_dump(pathinfo($testfile, PATHINFO_EXTENSION|PATHINFO_FILENAME|PATHINFO_DIRNAME));
var_dump(pathinfo($testfile, PATHINFO_EXTENSION|PATHINFO_FILENAME|PATHINFO_BASENAME));
var_dump(pathinfo($testfile, PATHINFO_EXTENSION|PATHINFO_FILENAME));
var_dump(pathinfo($testfile, PATHINFO_EXTENSION|PATHINFO_BASENAME));
var_dump(pathinfo($testfile, PATHINFO_FILENAME|PATHINFO_DIRNAME));
var_dump(pathinfo($testfile, PATHINFO_FILENAME|PATHINFO_BASENAME));
var_dump(pathinfo($testfile, PATHINFO_DIRNAME|PATHINFO_EXTENSION));
var_dump(pathinfo($testfile, PATHINFO_DIRNAME|PATHINFO_BASENAME));


?>
===DONE===