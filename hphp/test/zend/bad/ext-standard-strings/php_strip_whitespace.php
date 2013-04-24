<?php

$filename = dirname(__FILE__)."/php_strip_whitespace.dat";

var_dump(php_strip_whitespace());
var_dump(php_strip_whitespace($filename));

$data = '/* test comment */';
file_put_contents($filename, $data);
var_dump(php_strip_whitespace($filename));

$data = '<?php /* test comment */ ?>';
file_put_contents($filename, $data);
var_dump(php_strip_whitespace($filename));

$data = '<?php
/* test class */
class test {

	/* function foo () */

	function foo () /* {{{ */
	{


		echo $var; //does not exist
	}
	/* }}} */

}
?>';

file_put_contents($filename, $data);
var_dump(php_strip_whitespace($filename));

@unlink($filename);
echo "Done\n";
?>