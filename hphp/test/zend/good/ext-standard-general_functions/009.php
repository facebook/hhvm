<?php
function test($str) {
	$res = sha1($str)."\n";
	return $res;
} 
echo test("");
echo test("a");
echo test("abc");
echo test("message digest");
echo test("abcdefghijklmnopqrstuvwxyz");
echo test("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
echo test("12345678901234567890123456789012345678901234567890123456789012345678901234567890");
?>