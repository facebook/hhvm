<?php
/* Prototype  : mixed get_browser([string browser_name [, bool return_array]])
 * Description: Get information about the capabilities of a browser. 
 * If browser_name is omitted or null, HTTP_USER_AGENT is used. 
 * Returns an object by default; if return_array is true, returns an array. 
 *
 * Source code: ext/standard/browscap.c
 * Alias to functions: 
 */

$browsers = include dirname(__FILE__) . DIRECTORY_SEPARATOR . 'browsernames.inc';

echo "*** Testing get_browser() : error functionality ***\n";

/* Unknown browser uses defaults. */
var_dump( get_browser( 'foobar', true ) );

/* More than expected arguments */
var_dump( get_browser( 'foobar', true, 15 ) );

/* Some wrong parameters, no HTTP_USER_AGENT set */
var_dump( get_browser( null, 'foobar' ) );

?>
===DONE===