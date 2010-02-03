--TEST--
XMLReader: libxml2 XML Reader, next 
--SKIPIF--
<?php if (!extension_loaded("xmlreader")) print "skip"; ?>
--FILE--
<?php 
/* $Id: 009.phpt,v 1.1.2.2 2005/12/21 03:58:59 pajoye Exp $ */

$xmlstring = '<?xml version="1.0" encoding="UTF-8"?>
<books><book num="1"><test /></book><book num="2" /></books>';

$reader = new XMLReader();
$reader->XML($xmlstring);

// Only go through
$reader->read();
$reader->read();

$reader->next();
echo $reader->name;
echo " ";
echo $reader->getAttribute('num');
echo "\n";
?>
===DONE===
--EXPECTF--
book 2
===DONE===
