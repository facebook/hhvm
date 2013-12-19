<?php
$queries = array(
	"foo=abc#bar=def&fubar=ghi",
	"%2bfoo=def&-bar=jkl#+fubar",
	"  foo[]=abc&foo[]=def#foo[]=ghi#bar[]=#foo[]&fubar[]=="
);
function test($query) {
	$foo = '';
	$bar = '';
	$fubar = '';
	mb_parse_str($query, $array);
	var_dump($array);
	var_dump($foo);
	var_dump($bar);
	var_dump($fubar);
	mb_parse_str($query);
	var_dump($foo);
	var_dump($bar);
	var_dump($fubar);
}
foreach ($queries as $query) {
	test($query);
}
?>