<?php

function err($fmt) {
	if(!$fmt) {
		echo var_export(intl_get_error_message(), true)."\n";
	}
}

function crt($t, $l, $s) {
	switch(true) {
		case $t == "O":
			return new MessageFormatter($l, $s);
			break;
		case $t == "C":
			return MessageFormatter::create($l, $s);
			break;
		case $t == "P":
			return msgfmt_create($l, $s);
			break;
	}
}

$args = array(
	array(null, null),
	array("whatever", "{0,whatever}"),
	array(array(), array()),
	array("en", "{0,choice}"),
	array("fr", "{0,"),
	array("en_US", "\xD0"),
);

$fmt = new MessageFormatter();
err($fmt); 
$fmt = msgfmt_create();
err($fmt); 
$fmt = MessageFormatter::create();
err($fmt); 
$fmt = new MessageFormatter('en');
err($fmt); 
$fmt = msgfmt_create('en');
err($fmt); 
$fmt = MessageFormatter::create('en');
err($fmt); 

foreach($args as $arg) {
	$fmt = crt("O", $arg[0], $arg[1]);
	err($fmt);
	$fmt = crt("C", $arg[0], $arg[1]);
	err($fmt);
	$fmt = crt("P", $arg[0], $arg[1]);
	err($fmt);
}

?>