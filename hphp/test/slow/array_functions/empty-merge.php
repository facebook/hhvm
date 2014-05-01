<?php

function get_empty_array() {
  return extension_loaded('pdo') ? array() : array('wut');
}

function main() {
  $emp = get_empty_array();
  $full = array(new stdClass, 2);
  $merge = array_merge($emp, $full);
  return $merge;
}

var_dump(main());
