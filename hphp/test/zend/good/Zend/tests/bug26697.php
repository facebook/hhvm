<?php

function __autoload($name)
{
	echo __METHOD__ . "($name)\n";
	var_dump(class_exists('NotExistingClass'));
	echo __METHOD__ . "($name), done\n";
}

var_dump(class_exists('NotExistingClass'));

?>
===DONE===