<?php
/* Prototype  : array imap_alerts(void)
 * Description: Returns an array of all IMAP alerts that have been generated since the last page load or since the last imap_alerts() call, whichever came last. The alert stack is cleared after imap_alerts() is called. 
 * Source code: ext/imap/php_imap.c
 * Alias to functions: 
 */

echo "*** Testing imap_alerts() : error conditions ***\n";

// One argument
echo "\n-- Testing imap_alerts() function with one argument --\n";
$extra_arg = 10;;
var_dump( imap_alerts($extra_arg) );

?>
===DONE===