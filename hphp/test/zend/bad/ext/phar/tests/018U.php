<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';
$pname = 'phar://' . $fname;
$file = b"<?php
Phar::mapPhar('hio');
__HALT_COMPILER(); ?>";

$files = array();
$files['a'] = 'a';
$files['b/a'] = 'b';
include 'files/phar_test.inc';

include $fname;
$dir = opendir('phar://hio/');
while (false !== ($a = readdir($dir))) {
	var_dump($a);
	var_dump(is_dir('phar://hio/' . $a));
}
?>
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php'); ?>