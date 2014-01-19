<?php

$array = array('1' => array(2 => 'test'));
unset($array['1'][2]);
var_dump($array['1']);
