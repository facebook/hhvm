<?php

function shutdown() {
  var_dump("Script executed with success");
}

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
  undefined_function();
}

function destroy($id) {
  return true;
}

function gc($maxlifetime) {
  return true;
}


<<__EntryPoint>>
function main_register_shutdown_function() {
register_shutdown_function('shutdown');

session_set_save_handler('open', 'close', 'read', 'write', 'destroy', 'gc');
}
