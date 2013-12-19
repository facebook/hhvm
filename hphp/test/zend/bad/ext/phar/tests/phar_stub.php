<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';
$pname = 'phar://' . $fname;
$file = '<?php echo "first stub\n"; __HALT_COMPILER(); ?>';

$files = array();
$files['a'] = 'a';
$files['b'] = 'b';
$files['c'] = 'c';

include 'files/phar_test.inc';

$file = '<?php echo "first stub\n"; __HALT_COMPILER(); ?>';
$fp = fopen($fname, 'rb');
//// 1
echo fread($fp, strlen($file)) . "\n";
fclose($fp);
$phar = new Phar($fname);
$file = '<?php echo "second stub\n"; __HALT_COMPILER(); ?>';

//// 2
$phar->setStub($file);
$fp = fopen($fname, 'rb');
echo fread($fp, strlen($file)) . "\n";
fclose($fp);

$fname2 = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phartmp.php';
$file = b'<?php echo "third stub\n"; __HALT_COMPILER(); ?>';
$fp = fopen($fname2, 'wb');
fwrite($fp, $file);
fclose($fp);
$fp = fopen($fname2, 'rb');

//// 3
$phar->setStub($fp);
fclose($fp);

$fp = fopen($fname, 'rb');
echo fread($fp, strlen($file)) . "\n";
fclose($fp);

$fp = fopen($fname2, 'ab');
fwrite($fp, b'booya');
fclose($fp);
echo file_get_contents($fname2) . "\n";

$fp = fopen($fname2, 'rb');

//// 4
$phar->setStub($fp, strlen($file));
fclose($fp);

$fp = fopen($fname, 'rb');
echo fread($fp, strlen($file)) . "\n";
if (fread($fp, strlen('booya')) == 'booya') {
	echo 'failed - copied booya';
}
fclose($fp);
$phar['testing'] = 'hi';

// ensure stub is not overwritten
$fp = fopen($fname, 'rb');
echo fread($fp, strlen($file)) . "\n";
if (fread($fp, strlen('booya')) == 'booya') {
	echo 'failed - copied booya';
}
fclose($fp);

?>
===DONE===
<?php 
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phartmp.php');
__HALT_COMPILER();
?>