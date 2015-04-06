<?php

ob_start();

function s_open($path, $name) { return true; }
function s_close() { return true; }
function s_read($id) { return ''; }
function s_write($id, $data) {
  var_dump('s_write');
  var_dump($id);
  var_dump($data);
  return true;
}
function s_destroy($id) { return true; }
function s_gc($t) { return true; }

session_set_save_handler(
  's_open',
  's_close',
  's_read',
  's_write',
  's_destroy',
  's_gc'
);

session_start();

$_SESSION['foo'] = 10;
$_SESSION['bar'] = 20;

var_dump($_SESSION);
session_write_close();

session_start();
session_destroy();

session_start();
