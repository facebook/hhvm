<?php

ob_start();

function open($save_path, $session_name) {
    return true;
}

function close() {
	echo "close: goodbye cruel world\n";
	exit;
}

function read($id) {
	return '';
}

function write($id, $session_data) {
	echo "write: goodbye cruel world\n";
	undefined_function();
}

function destroy($id) {
    return true;
}

function gc($maxlifetime) {
    return true;
}

session_set_save_handler('open', 'close', 'read', 'write', 'destroy', 'gc');
session_start();

?>