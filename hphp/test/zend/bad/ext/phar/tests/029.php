<?php
$fname1 = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.1.phar.php';
$fname2 = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.2.phar.php';
$fname = $fname1;
$alias = '';
$pname = 'phar://hio';
$file = '<?php include "' . $pname . '/a.php"; __HALT_COMPILER(); ?>';

$files = array();
$files['a.php']   = '<?php echo "This is a\n"; include "'.$pname.'/b.php"; ?>';      
$files['b.php']   = '<?php echo "This is b\n"; include "'.$pname.'/b/c.php"; ?>';    
$files['b/c.php'] = '<?php echo "This is b/c\n"; include "'.$pname.'/b/d.php"; ?>';  
$files['b/d.php'] = '<?php echo "This is b/d\n"; include "'.$pname.'/e.php"; ?>';    
$files['e.php']   = '<?php echo "This is e\n"; ?>';                                  

include 'files/phar_test.inc';

copy($fname1, $fname2);

var_dump(Phar::loadPhar($fname1, 'hio'));
var_dump(Phar::loadPhar($fname1, 'copy'));
$a = new Phar($fname1);
try
{
	var_dump(Phar::loadPhar($fname2, 'copy'));
}
catch (Exception $e)
{
	echo $e->getMessage() . "\n";
}

?>
===DONE===
<?php error_reporting(0); ?>
<?php 
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.1.phar.php');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.2.phar.php');
?>