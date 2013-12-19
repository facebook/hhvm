<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';
$pname = 'phar://' . $fname;
$file = "<?php
Phar::mapPhar('hio');
var_dump(__FILE__);
var_dump(substr(__FILE__, 0, 4) != 'phar');
__HALT_COMPILER(); ?>";

$files = array();
$files['a'] = 'abc';
include 'files/phar_test.inc';

include $pname;
$dir = opendir('phar://hio');
?>
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php'); ?>