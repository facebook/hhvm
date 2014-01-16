<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.zip';
$pname = 'phar://' . $fname;

copy(dirname(__FILE__) . '/files/zip.zip', $fname);
include $fname;
?>
===DONE===
<?php 
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.zip');
__HALT_COMPILER();
?>