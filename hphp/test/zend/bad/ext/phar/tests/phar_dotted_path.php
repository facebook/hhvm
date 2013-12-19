<?php

$dir = dirname(__FILE__) . '/broken.dirname';
mkdir($dir, 0777);

$fname = $dir . '/dotted_path.phar';
$stub = Phar::createDefaultStub();
$file = $stub;

$files = array();
$files['a'] = 'this is a';
$files['b'] = 'this is b';

include 'files/phar_test.inc';

$phar = new Phar($fname);

foreach ($phar as $entry) {
    echo file_get_contents($entry)."\n";
}

?>
===DONE===
<?php
unlink(dirname(__FILE__) . '/broken.dirname/dotted_path.phar');
rmdir(dirname(__FILE__) . '/broken.dirname');
?>