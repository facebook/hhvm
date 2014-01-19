<?php

ob_start();
/* 
 * Prototype : string session_save_path([string $path])
 * Description : Get and/or set the current session save path
 * Source code : ext/session/session.c 
 */

echo "*** Testing session_save_path() : variation ***\n";
$directory = dirname(__FILE__);
$sessions = ($directory."/sessions");

chdir($directory);
ini_set('open_basedir', '.');
// Delete the existing directory
if (file_exists($sessions) === TRUE) {
	@rmdir($sessions);
}

var_dump(mkdir($sessions));
var_dump(chdir($sessions));
ini_set("session.save_path", $directory);
var_dump(session_save_path());
var_dump(rmdir($sessions));

echo "Done";
ob_end_flush();
?>
$directory = dirname(__FILE__);
$sessions = ($directory."/sessions");
var_dump(rmdir($sessions));