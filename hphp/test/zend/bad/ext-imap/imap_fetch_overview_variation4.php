<?php
/* Prototype  : array imap_fetch_overview(resource $stream_id, int $msg_no [, int $options])
 * Description: Read an overview of the information in the headers 
 * of the given message sequence 
 * Source code: ext/imap/php_imap.c
 */

/*
 * Pass different resource types to imap_fetch_overview() to test behaviour
 */

echo "*** Testing imap_fetch_overview() : usage variations ***\n";

echo "\n-- File Resource opened with fopen() --\n";
var_dump($file_pointer = fopen(__FILE__, 'r+'));
var_dump(imap_fetch_overview($file_pointer, 1));
fclose($file_pointer);

echo "\n-- Directory Resource opened with opendir() --\n";
var_dump($dir_handle = opendir(dirname(__FILE__)));
var_dump(imap_fetch_overview($dir_handle, 1));
closedir($dir_handle);
?>
===DONE===