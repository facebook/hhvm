<?php
echo file_get_contents('phar://' . dirname(__FILE__) . '/files/links.phar.tar/link2');
echo file_get_contents('phar://' . dirname(__FILE__) . '/files/links.phar.tar/link1');
echo file_get_contents('phar://' . dirname(__FILE__) . '/files/links.phar.tar/testit.txt');

$a = fopen('phar://' . dirname(__FILE__) . '/files/links.phar.tar/link2', 'r');
fseek($a, 3);
echo fread($a, 10);
?>
===DONE===