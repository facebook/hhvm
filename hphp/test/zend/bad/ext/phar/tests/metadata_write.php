<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';
$pname = 'phar://' . $fname;
$file = "<?php __HALT_COMPILER(); ?>";

$files = array();
$files['a'] = array('cont' => 'a');
$files['b'] = array('cont' => 'b', 'meta' => 'hi there');
$files['c'] = array('cont' => 'c', 'meta' => array('hi', 'there'));
$files['d'] = array('cont' => 'd', 'meta' => array('hi'=>'there','foo'=>'bar'));
include 'files/phar_test.inc';

foreach($files as $name => $cont) {
	var_dump(file_get_contents($pname.'/'.$name));
}

$phar = new Phar($fname);
$phar['a']->setMetadata(42);
$phar['b']->setMetadata(NULL);
$phar['c']->setMetadata(array(25, 'foo'=>'bar'));
$phar['d']->setMetadata(true);

foreach($files as $name => $cont) {
	var_dump($phar[$name]->getMetadata());
}

unset($phar);

foreach($files as $name => $cont) {
	var_dump(file_get_contents($pname.'/'.$name));
}
?>
===DONE===
<?php error_reporting(0); ?>
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php'); ?>