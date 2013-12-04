<?php
var_dump(file_exists('phar://' . dirname(__FILE__) . '/files/write4.phar/testit.txt'));
Phar::mount('phar://' . dirname(__FILE__) . '/files/write4.phar/testit.txt', 'phar://' . dirname(__FILE__) . '/files/write4.phar/tobemounted');
var_dump(file_exists('phar://' . dirname(__FILE__) . '/files/write4.phar/testit.txt'), file_get_contents('phar://' . dirname(__FILE__) . '/files/write4.phar/testit.txt'));
?>
===DONE===