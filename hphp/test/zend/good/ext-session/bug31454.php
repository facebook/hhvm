<?php

session_set_save_handler(
	array(&$arf, 'open'),
	array(&$arf, 'close'),
	array(&$arf, 'read'),
	array(&$arf, 'write'),
	array(&$arf, 'destroy'),
	array(&$arf, 'gc'));
	
echo "Done\n";
?>