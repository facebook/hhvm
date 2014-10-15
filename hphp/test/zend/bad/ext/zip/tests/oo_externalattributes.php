<?php

$name = __DIR__ . '/withattr.zip';

echo "== Set\n";
$zip = new ZipArchive;
$r = $zip->open($name, ZIPARCHIVE::CREATE);
$zip->addFromString('foo.txt', 'foo');
$zip->addFromString('bar.txt', 'bar');
var_dump($zip->setExternalAttributesName('foo.txt', ZipArchive::OPSYS_UNIX, 123));
$id = $zip->locateName('bar.txt');
var_dump($zip->setExternalAttributesIndex($id, ZipArchive::OPSYS_VFAT, 234));
$zip->close();

echo "== Get\n";
$r = $zip->open($name);
var_dump($zip->getExternalAttributesName('foo.txt', $a, $b), $a, $b);
$id = $zip->locateName('bar.txt');
var_dump($zip->getExternalAttributesIndex($id, $a, $b), $a, $b);

echo "== Set\n";
var_dump($zip->setExternalAttributesName('foo.txt', ZipArchive::OPSYS_DOS, 345));
var_dump($zip->setExternalAttributesIndex($id, ZipArchive::OPSYS_AMIGA, 456));
echo "== Get changed\n";
var_dump($zip->getExternalAttributesName('foo.txt', $a, $b), $a, $b);
var_dump($zip->getExternalAttributesIndex($id, $a, $b), $a, $b);
echo "== Get unchanged\n";
var_dump($zip->getExternalAttributesName('foo.txt', $a, $b, ZipArchive::FL_UNCHANGED), $a, $b);
var_dump($zip->getExternalAttributesIndex($id, $a, $b, ZipArchive::FL_UNCHANGED), $a, $b);

$zip->close();
?>
== Done
<?php error_reporting(0); ?>
<?php
$name = __DIR__ . '/withattr.zip';
@unlink($name);
?>