<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.zip.php';
$f2name = dirname(__FILE__) . '/files/metadata.phar.zip';
$pname = 'phar://' . $fname;
$p2name = 'phar://' . $f2name;

$file = "<?php __HALT_COMPILER(); ?>";

$files = array();
$files['a'] = array('cont' => 'a');
$files['b'] = array('cont' => 'b', 'meta' => 'hi there');
$files['c'] = array('cont' => 'c', 'meta' => array('hi', 'there'));
$files['d'] = array('cont' => 'd', 'meta' => array('hi'=>'there','foo'=>'bar'));

foreach($files as $name => $cont) {
	var_dump(file_get_contents($p2name.'/'.$name));
}

copy($f2name, $fname);
$phar = new Phar($fname);
$phar->startBuffering();
$phar['a']->setMetadata(42);
$phar['b']->setMetadata(NULL);
$phar['c']->setMetadata(array(25, 'foo'=>'bar'));
$phar['d']->setMetadata(true);
$phar->setMetadata('hi');

foreach($files as $name => $cont) {
	var_dump($phar[$name]->getMetadata());
}
$phar->stopBuffering();

unset($phar);

$phar = new Phar($fname);

foreach($files as $name => $cont) {
	var_dump(file_get_contents($pname.'/'.$name));
}

foreach($files as $name => $cont) {
	var_dump($phar[$name]->getMetadata());
}
var_dump($phar->getMetadata());
?>
===DONE===
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.zip.php'); ?>