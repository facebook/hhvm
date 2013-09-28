<?php
namespace test\ns1;

function foo($x = INI_ALL) {
	var_dump($x);
}
foo();