<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar';
$pname = 'phar://' . $fname;

$a = new Phar($fname);
$a['a/subdir/here'] = 'hi';

$b = new PharFileInfo($pname . '/a/subdir');
unset($b);

$b = new PharFileInfo($pname . '/a/subdir/here');
unset($b);
?>
===DONE===
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar'); ?>