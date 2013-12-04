<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';
$pname = 'phar://hio';
$file = '<?php include "' . $pname . '/a.php"; __HALT_COMPILER(); ?>';
$alias = '';

$files = array();
$files['a.php']   = '<?php echo "This is a\n"; include "'.$pname.'/b.php"; ?>';      
$files['b.php']   = '<?php echo "This is b\n"; include "'.$pname.'/b/c.php"; ?>';    
$files['b/c.php'] = '<?php echo "This is b/c\n"; include "'.$pname.'/b/d.php"; ?>';  
$files['b/d.php'] = '<?php echo "This is b/d\n"; include "'.$pname.'/e.php"; ?>';    
$files['e.php']   = '<?php echo "This is e\n"; ?>';                                  

include 'files/phar_test.inc';

Phar::loadPhar($fname, 'hio');

include $fname;

echo "======\n";

include $pname . '/a.php';

?>
===DONE===
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php'); ?>