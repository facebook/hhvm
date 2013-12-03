<?php
 file_put_contents('test.php',str_repeat('passed, ',1024));
 require('./test.php');
 unlink('test.php');
unlink('test.php');
?>