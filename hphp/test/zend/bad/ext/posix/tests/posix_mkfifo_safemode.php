<?php
var_dump(posix_mkfifo('/tmp/foobar', 0644));

$dir = dirname(__FILE__) . '/foo';
mkdir ($dir);
var_dump(posix_mkfifo($dir . '/bar', 0644));
?>
===DONE===
<?php
$dir = dirname(__FILE__) . '/foo';
unlink($dir . '/bar');
rmdir($dir);
?>