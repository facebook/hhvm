<?php
$queries = array(
	"foo=abc&bar=def",
	"%2bfoo=def&-bar=jkl",
	"foo[]=abc&foo[]=def&foo[]=ghi&bar[]=jkl"
);
function test($query) {
	$foo = '';
	$bar = '';
	mb_parse_str($query, $array);
	var_dump($array);
	var_dump($foo);
	var_dump($bar);
	mb_parse_str($query);
	var_dump($foo);
	var_dump($bar);
}
foreach ($queries as $query) {
	test($query);
}
?>