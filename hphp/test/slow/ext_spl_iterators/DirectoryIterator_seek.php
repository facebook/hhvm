<?php

$dit = new DirectoryIterator(__DIR__);

$dit->seek(5);
echo $dit->key(), PHP_EOL;
$dit->seek(4);
echo $dit->key(), PHP_EOL;
