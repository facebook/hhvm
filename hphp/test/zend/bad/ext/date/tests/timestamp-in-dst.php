<?php
error_reporting(E_ALL & ~E_WARNING); // hide e_warning warning about timezones
var_dump( date_create( '@1202996091' )->format( 'c' ) );
?>