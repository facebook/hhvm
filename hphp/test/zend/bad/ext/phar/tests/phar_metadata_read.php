<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';
$pname = 'phar://' . $fname;
$file = "<?php __HALT_COMPILER(); ?>";

$files = array();
$pmeta = 'hi there';
$files['a'] = array('cont' => 'a');
$files['b'] = array('cont' => 'b');
$files['c'] = array('cont' => 'c', 'meta' => array('hi', 'there'));
$files['d'] = array('cont' => 'd', 'meta' => array('hi'=>'there','foo'=>'bar'));
include 'files/phar_test.inc';

foreach($files as $name => $cont) {
	var_dump(file_get_contents($pname.'/'.$name));
}

$phar = new Phar($fname);
var_dump($phar->hasMetaData());
var_dump($phar->getMetaData());
var_dump($phar->delMetaData());
var_dump($phar->getMetaData());
var_dump($phar->delMetaData());
var_dump($phar->getMetaData());
foreach($files as $name => $cont) {
	echo "  meta $name\n";
	var_dump($phar[$name]->hasMetadata());
	var_dump($phar[$name]->getMetadata());
	var_dump($phar[$name]->delMetadata());
	var_dump($phar[$name]->getMetadata());
}

unset($phar);

foreach($files as $name => $cont) {
	var_dump(file_get_contents($pname.'/'.$name));
}
?>
===DONE===
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php'); ?>