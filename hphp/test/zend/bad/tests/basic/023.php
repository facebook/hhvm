<?php
$_COOKIE = http_parse_cookie("c o o k i e=value; c o o k i e= v a l u e ;;c%20o+o k+i%20e=v;name="value","value",UEhQIQ==;UEhQIQ==foo");
_filter_snapshot_globals();

var_dump($_COOKIE);
?>
