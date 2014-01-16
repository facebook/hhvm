<?php

ob_start();

/* 
 * Prototype : string session_save_path([string $path])
 * Description : Get and/or set the current session save path
 * Source code : ext/session/session.c 
 */

echo "*** Testing session_save_path() : variation ***\n";
$initdir = getcwd();
$sessions = ($initdir."/sessions");

chdir($initdir);

// Delete the existing directory
if (file_exists($sessions) === TRUE) {
	@rmdir($sessions);
}

var_dump(mkdir($sessions));
var_dump(chdir($sessions));
ini_set("session.save_path", $initdir);
var_dump(session_save_path());
var_dump(session_start());
var_dump(session_save_path());
var_dump(session_destroy());
var_dump(session_save_path());
var_dump(rmdir($sessions));

echo "Done";
ob_end_flush();
?>
$initdir = getcwd();
$sessions = ($initdir."/sessions");
var_dump(rmdir($sessions));