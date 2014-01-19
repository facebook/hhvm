<?php
	$simple = simplexml_load_file(dirname(__FILE__)."/book.xml");
	$simple->addAttribute('type','novels');
	
	var_dump($simple->attributes());
	echo "Done";
?>