<?php
function filter_cb($var)
{
  return 1;
}

$data = array ('bar' => array ('fu<script>bar', 'bar<script>fu') );
var_dump(filter_var($data, FILTER_SANITIZE_STRING, FILTER_FORCE_ARRAY));
var_dump($data);
var_dump(filter_var($data, FILTER_CALLBACK, array('options' => 'filter_cb')));
var_dump($data);
var_dump(filter_var_array($data, array('bar' => array('filter' => FILTER_CALLBACK, 'options' => 'filter_cb'))));
var_dump($data);