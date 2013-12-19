<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';
$pname = 'phar://' . $fname;
$file = '<?php include "' . $pname . '/a.php"; __HALT_COMPILER(); ?>';

$files = array();
$files['a.php']   = '<?php echo "This is a\n"; include \''.$pname.'/b.php\'; ?>';
$files['b.php']   = '<?php echo "This is b\n"; include \''.$pname.'/b/c.php\'; ?>';
$files['b/c.php'] = '<?php echo "This is b/c\n"; include \''.$pname.'/b/d.php\'; ?>';
$files['b/d.php'] = '<?php echo "This is b/d\n"; include \''.$pname.'/e.php\'; ?>';
$files['e.php']   = '<?php echo "This is e\n"; ?>';
$files['.phar/test'] = '<?php bad boy ?>';

include 'files/phar_test.inc';

Phar::loadPhar($fname);

require $pname . '/a.php';

$p = new Phar($fname);
var_dump(isset($p['.phar/test']));
try {
$p['.phar/test'];
} catch (Exception $e) {
echo $e->getMessage(),"\n";
}
?>
===DONE===
<?php 
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php');
?>