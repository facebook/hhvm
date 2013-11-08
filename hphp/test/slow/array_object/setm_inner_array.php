<?php

$obj = new ArrayObject(array('key' => array()));
$obj['key']['other_key'] = 'other_val';

var_dump($obj['key']);
