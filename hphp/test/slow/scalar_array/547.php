<?php

define('VALUE', 1);
function func($params) {
 var_dump($params);
}
func(array('key' => @VALUE));
