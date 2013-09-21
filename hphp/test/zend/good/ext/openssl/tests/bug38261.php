<?php
$cert = false;
class test {
	function __toString() {
		return "test object";
	}
}
$t = new test;

var_dump(openssl_x509_parse("foo"));
var_dump(openssl_x509_parse($t));
var_dump(openssl_x509_parse(array()));
var_dump(openssl_x509_parse());
var_dump(openssl_x509_parse($cert));
var_dump(openssl_x509_parse(new stdClass));

?>