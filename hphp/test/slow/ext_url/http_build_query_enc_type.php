<?php
var_dump(PHP_QUERY_RFC1738);
var_dump(PHP_QUERY_RFC3986);

$data = array('foo bar' => 'herp derp');
var_dump(http_build_query($data, null, null, PHP_QUERY_RFC1738));
var_dump(http_build_query($data, null, null, PHP_QUERY_RFC3986));
var_dump(http_build_query($data, null, null, 42 /* invalid */));
