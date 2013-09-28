<?php
$date = date_create('asdfasdf');

if ($date instanceof DateTime) {
	echo "this is wrong, should be bool";
}

var_dump( $date );
var_dump( DateTime::getLastErrors() );
var_dump( date_get_last_errors() );
?>