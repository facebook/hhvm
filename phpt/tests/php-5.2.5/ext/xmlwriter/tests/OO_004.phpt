--TEST--
XMLWriter: libxml2 XML Writer, file buffer, flush
--SKIPIF--
<?php if (!extension_loaded("xmlwriter")) print "skip"; ?>
--FILE--
<?php 
/* $Id: OO_004.phpt,v 1.3.2.2 2005/12/02 02:05:26 iliaa Exp $ */

$doc_dest = '001.xml';
$xw = new XMLWriter();
$xw->openUri($doc_dest);
$xw->startDocument('1.0', 'UTF-8');
$xw->startElement("tag1");

$xw->startPi("PHP");
$xw->text('echo $a;');
$xw->endPi();
$xw->endDocument();

// Force to write and empty the buffer
$xw->flush(true);
$md5_out = md5_file($doc_dest);
$md5_res = md5('<?xml version="1.0" encoding="UTF-8"?>
<tag1><?PHP echo $a;?></tag1>
');
unset($xw);
unlink('001.xml');
if ($md5_out != $md5_res) {
	echo "failed: $md5_res != $md5_out\n";
} else {
	echo "ok.\n";
}
?>
===DONE===
--EXPECT--
ok.
===DONE===
