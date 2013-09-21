<?php

error_reporting(E_ALL);

foreach (($a = array('a' => array('a' => &$a))) as $a) {
	var_dump($a);
}

?>