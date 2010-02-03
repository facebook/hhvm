--TEST--
XMLWriter: libxml2 XML Writer, startDTD/writeElementNS 
--SKIPIF--
<?php 
if (!extension_loaded("xmlwriter")) die("skip"); 
?>
--FILE--
<?php 
/* $Id: OO_006.phpt,v 1.1.2.3 2005/12/12 21:21:11 tony2001 Exp $ */

$doc_dest = '001.xml';
$xw = new XMLWriter();
$xw->openUri($doc_dest);
$xw->startDtd('foo', NULL, 'urn:bar');
$xw->endDtd();
$xw->startElement('foo');
$xw->writeElementNS('foo', 'bar', 'urn:foo', 'dummy content');
$xw->endElement();

// Force to write and empty the buffer
$output_bytes = $xw->flush(true);
echo file_get_contents($doc_dest);
unset($xw);
unlink('001.xml');
?>
--EXPECT--
<!DOCTYPE foo SYSTEM "urn:bar"><foo><foo:bar xmlns:foo="urn:foo">dummy content</foo:bar></foo>
