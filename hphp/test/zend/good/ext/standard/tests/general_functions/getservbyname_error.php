<?php
$service = "www";
$protocol = "tcp"; 
$extra_arg = 12;
var_dump(getservbyname($service, $protocol, $extra_arg ) );
var_dump(getservbyname($service));
?>