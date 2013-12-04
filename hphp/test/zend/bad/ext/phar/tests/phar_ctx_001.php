<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';
$pname = 'phar://' . $fname;
$file = '<?php __HALT_COMPILER(); ?>';

$files = array();
$files['a'] = 'a';
$files['b'] = 'b';
$files['c'] = 'c';

include 'files/phar_test.inc';

$phar = new Phar($fname);

var_dump(file_get_contents($pname . '/a'));
var_dump($phar['a']->isCompressed());
var_dump(file_get_contents($pname . '/b'));
var_dump($phar['b']->isCompressed());
var_dump(file_get_contents($pname . '/c'));
var_dump($phar['c']->isCompressed());

$context = stream_context_create(array('phar'=> array('compress'=>Phar::GZ, 'metadata' => array(2, b'hi' => 3))));
$context2 = stream_context_create(array('phar' => array('metadata' => array(4))));

file_put_contents($pname . '/a', b'new a', 0); // no compression
file_put_contents($pname . '/b', b'new b', 0, $context);
file_put_contents($pname . '/d', b'new d', 0, $context2);

$phar = new Phar($fname);
var_dump(file_get_contents($pname . '/a'));
var_dump($phar['a']->isCompressed());
var_dump($phar['a']->getMetaData());
var_dump(file_get_contents($pname . '/b'));
var_dump($phar['b']->isCompressed());
var_dump($phar['b']->getMetaData());
var_dump(file_get_contents($pname . '/c'));
var_dump($phar['c']->isCompressed());
var_dump($phar['c']->getMetaData());
var_dump(file_get_contents($pname . '/d'));
var_dump($phar['d']->isCompressed());
var_dump($phar['d']->getMetaData());
$context2 = stream_context_create(array('phar' => array('metadata' => array(4))));
$fp = fopen($pname . '/b', 'r+', 0, $context2);
fclose($fp);
?>
==AFTER==
<?php
var_dump(file_get_contents($pname . '/b'));
var_dump($phar['b']->isCompressed());
var_dump($phar['b']->getMetaData());
?>
===DONE===
<?php 
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php');
?>