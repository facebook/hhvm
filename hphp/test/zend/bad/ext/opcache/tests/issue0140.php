<?php
define("FILENAME", dirname(__FILE__) . "/issuer0140.inc.php");
file_put_contents(FILENAME, "1\n");

var_dump(is_readable(FILENAME));
include(FILENAME);
var_dump(filemtime(FILENAME));

sleep(2);
file_put_contents(FILENAME, "2\n");

var_dump(is_readable(FILENAME));
include(FILENAME);
var_dump(filemtime(FILENAME));

sleep(2);
unlink(FILENAME);

var_dump(is_readable(FILENAME));
var_dump(@include(FILENAME));
var_dump(@filemtime(FILENAME));
?>