<?php
$port = 80;
$protocol = "tcp"; 
$extra_arg = 12;
var_dump(getservbyport( $port, $protocol, $extra_arg ) );
var_dump(getservbyport($port));
?>