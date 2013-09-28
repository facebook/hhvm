<?php

$filename = dirname(__FILE__)."/symlink.dat";
$link = dirname(__FILE__)."/symlink.link";

var_dump(symlink($filename, $link));
var_dump(readlink($link));
var_dump(linkinfo($link));
@unlink($link);

var_dump(readlink($link));
var_dump(linkinfo($link));

touch($filename);
var_dump(symlink($filename, dirname(__FILE__)));
@unlink($link);

var_dump(symlink($filename, $link));
@unlink($link);

touch($link);
var_dump(symlink($filename, $link));
@unlink($link);

var_dump(link($filename, $link));
@unlink($filename);

var_dump(link($filename, $link));
@unlink($link);

var_dump(symlink(".", "."));
var_dump(link(".", "."));
var_dump(readlink("."));
var_dump(linkinfo("."));

echo "Done\n";
?>