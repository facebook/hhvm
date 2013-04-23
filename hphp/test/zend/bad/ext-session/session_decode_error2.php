<?php

ob_start();

/* 
 * Prototype : string session_decode(void)
 * Description : Decodes session data from a string
 * Source code : ext/session/session.c 
 */

echo "*** Testing session_decode() : error functionality ***\n";
$data = "foo|a:3:{i:0;i:1;i:1;i:2;i:2;i:3;}guff|R:1;blah|R:1;";

var_dump(session_start());
for($index = 0; $index < strlen($data); $index++) {
    echo "\n-- Iteration $index --\n";
    $encoded = substr($data, 0, $index);
    var_dump(session_decode($encoded));
    var_dump($_SESSION);
};

var_dump(session_destroy());
echo "Done";
ob_end_flush();
?>