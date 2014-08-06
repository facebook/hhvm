<?php
$_COOKIE = http_parse_cookie("cookie1=val1  ; cookie2=val2%20; cookie3=val 3.; cookie 4= value 4 %3B; cookie1=bogus; %20cookie1=ignore;+cookie1=ignore;cookie1;cookie  5=%20 value; cookie%206=þæö;cookie+7=;$cookie.8;cookie-9=1;;;- & % $cookie 10=10");
_filter_snapshot_globals();

var_dump($_COOKIE);
?>
