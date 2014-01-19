<?php
var_dump(filter_var("192.168.0.1", FILTER_VALIDATE_IP));
var_dump(filter_var("192.168.0.1.1", FILTER_VALIDATE_IP));
var_dump(filter_var("::1", FILTER_VALIDATE_IP));
var_dump(filter_var("fe00::0", FILTER_VALIDATE_IP));
var_dump(filter_var("::123456", FILTER_VALIDATE_IP));
var_dump(filter_var("::1::b", FILTER_VALIDATE_IP));
var_dump(filter_var("127.0.0.1", FILTER_VALIDATE_IP));
var_dump(filter_var("192.168.0.1", FILTER_VALIDATE_IP, FILTER_FLAG_NO_PRIV_RANGE));
var_dump(filter_var("192.0.34.166", FILTER_VALIDATE_IP, FILTER_FLAG_NO_PRIV_RANGE));
var_dump(filter_var("127.0.0.1", FILTER_VALIDATE_IP, FILTER_FLAG_NO_RES_RANGE));
var_dump(filter_var("192.0.0.1", FILTER_VALIDATE_IP, FILTER_FLAG_NO_RES_RANGE));
var_dump(filter_var("100.64.0.0", FILTER_VALIDATE_IP, FILTER_FLAG_NO_RES_RANGE));
var_dump(filter_var("100.127.255.255", FILTER_VALIDATE_IP, FILTER_FLAG_NO_RES_RANGE));
var_dump(filter_var("192.0.34.166", FILTER_VALIDATE_IP));
var_dump(filter_var("256.1237.123.1", FILTER_VALIDATE_IP));
var_dump(filter_var("255.255.255.255", FILTER_VALIDATE_IP));
var_dump(filter_var("255.255.255.255", FILTER_VALIDATE_IP, FILTER_FLAG_NO_RES_RANGE));
var_dump(filter_var("", FILTER_VALIDATE_IP));
var_dump(filter_var(-1, FILTER_VALIDATE_IP));
var_dump(filter_var("::1", FILTER_VALIDATE_IP, FILTER_FLAG_IPV4));
var_dump(filter_var("127.0.0.1", FILTER_VALIDATE_IP, FILTER_FLAG_IPV6));
var_dump(filter_var("::1", FILTER_VALIDATE_IP, FILTER_FLAG_IPV6));
var_dump(filter_var("::1", FILTER_VALIDATE_IP, FILTER_FLAG_IPV6 | FILTER_FLAG_NO_RES_RANGE));
var_dump(filter_var("127.0.0.1", FILTER_VALIDATE_IP, FILTER_FLAG_IPV4));
echo "Done\n";
?>