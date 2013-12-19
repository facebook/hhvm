<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.1.phar.php';
$fname2 = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.tar';
$pname = 'phar://hio';
$file = '<?php include "' . $pname . '/a.php"; __HALT_COMPILER(); ?>';

$files = array();
$files['a.php']   = '<?php echo "This is a\n"; include "'.$pname.'/b.php"; ?>';
$files['dir/'] = '';
$hasdir = 1;
include 'files/phar_test.inc';
$a = new Phar($fname);
$b = new PharData($fname2);
$b['test'] = 'hi';

var_dump($a['a.php']->isWritable());
var_dump($a['a.php']->isReadable());
$a['a.php']->chmod(000);
var_dump($a['a.php']->isWritable());
var_dump($a['a.php']->isReadable());
$a['a.php']->chmod(0666);
var_dump($a['a.php']->isWritable());
var_dump($a['a.php']->isReadable());
ini_set('phar.readonly',1);
clearstatcache();
var_dump($a['a.php']->isWritable());
var_dump($a['a.php']->isReadable());
ini_set('phar.readonly',0);
clearstatcache();
var_dump($a['a.php']->isWritable());
var_dump($a['a.php']->isReadable());
?>
archive
<?php
ini_set('phar.readonly',0);
$p = new Phar('doesnotexisthere.phar');
var_dump($p->isWritable());
clearstatcache();
var_dump($a->isWritable());
var_dump($b->isWritable());
ini_set('phar.readonly',1);
clearstatcache();
var_dump($a->isWritable());
var_dump($b->isWritable());
chmod($fname2, 000);
clearstatcache();
var_dump($a->isWritable());
var_dump($b->isWritable());
chmod($fname2, 0666);
?>
===DONE===
<?php 
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.1.phar.php');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.tar');
?>