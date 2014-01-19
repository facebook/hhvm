<?php

ob_start();

function open($save_path, $session_name) {
    return true;
}

function close() {
	echo "close: goodbye cruel world\n";
}

function read($id) {
	return '';
}

function write($id, $session_data) {
	echo "write: goodbye cruel world\n";
	throw new Exception;
}

function destroy($id) {
    return true;
}

function gc($maxlifetime) {
    return true;
}

session_set_save_handler('open', 'close', 'read', 'write', 'destroy', 'gc');
session_start();
session_write_close();
echo "um, hi\n";

?>