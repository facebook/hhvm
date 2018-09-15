<?php
function func($params) {
 var_dump($params);
}


<<__EntryPoint>>
function main_547() {
define('VALUE', 1);
func(array('key' => @VALUE));
}
