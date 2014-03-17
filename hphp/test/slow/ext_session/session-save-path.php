<?php

define('DS', DIRECTORY_SEPARATOR);

$result = array();
foreach (array('a', 'b') as $sess) {
  $tmp = sys_get_temp_dir() . DS . $sess;
  if (is_dir($tmp) || mkdir($tmp)) {
    session_save_path($tmp);
    session_id($sess);
    session_start();
    $file = $tmp . DS . 'sess_' . $sess;
    $result[] = file_exists($file);
    unlink($file);
    session_destroy();
  }
}

var_dump($result);

