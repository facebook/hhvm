<?php

$array_variables = array(  array(),  array(NULL),  array());
foreach ($array_variables as $array_var) {
  $keys = array_keys($array_var);
  foreach ($keys as $key_value) {
    echo $key_value;
  }
}
