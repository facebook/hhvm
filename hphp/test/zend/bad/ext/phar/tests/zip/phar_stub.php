<?php
$fname2 = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.2.phar.zip.php';
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.zip.php';
$pname = 'phar://' . $fname;
$pname2 = 'phar://' . $fname2;

$p = new Phar($pname2);
$p->setStub('<?php echo "first stub\n"; __HALT_COMPILER(); ?>');
$p['a'] = 'a';
$p['b'] = 'b';
$p['c'] = 'c';
copy($fname2, $fname);

$phar = new Phar($fname);
echo $phar->getStub();

$file = b'<?php echo "second stub\n"; __HALT_COMPILER(); ?>';

//// 2
$phar->setStub($file);
echo $phar->getStub();

$fname3 = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phartmp.php';
$file = b'<?php echo "third stub\n"; __HALT_COMPILER(); ?>';
$fp = fopen($fname3, 'wb');
fwrite($fp, $file);
fclose($fp);
$fp = fopen($fname3, 'rb');

//// 3
$phar->setStub($fp);
fclose($fp);

echo $phar->getStub();

$fp = fopen($fname3, 'ab');
fwrite($fp, b'booya');
fclose($fp);
echo file_get_contents($fname3) . "\n";

$fp = fopen($fname3, 'rb');

//// 4
$phar->setStub($fp, strlen($file));
fclose($fp);
echo $phar->getStub();

$phar['testing'] = 'hi';

echo $phar->getStub();
?>
===DONE===
<?php 
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.zip.php');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.2.phar.zip.php');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phartmp.php');
__HALT_COMPILER();
?>