<?php
$pname = 'phar://' . dirname(__FILE__) . '/files/nophar.phar';
var_dump(file_get_contents($pname . '/b/c.php'));
$a = opendir($pname);
while (false !== ($b = readdir($a))) {
var_dump($b);
}
foreach (new RecursiveIteratorIterator(new Phar($pname)) as $f) {
	var_dump($f->getPathName());
}
var_dump(is_dir($pname . '/b'));
var_dump(is_dir($pname . '/d'));
var_dump(is_dir($pname . '/b/c.php'));
?>
===DONE===