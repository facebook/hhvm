<?php
$file = __FILE__;
$s = new SplFileObject( $file );
var_dump($fi = $s->getFileInfo(), (string)$fi);

$d = new SplFileInfo( __DIR__ );
echo "\n";
var_dump($fi = $d->getFileInfo(), (string)$fi);
$d = new SplFileInfo( __DIR__."/" );
echo "\n";
var_dump($fi = $d->getFileInfo(), (string)$fi);
?>