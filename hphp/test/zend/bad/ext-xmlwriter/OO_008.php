<?php 
/* $Id$ */

$xw = new XMLWriter();
$xw->openMemory();
$xw->setIndent(TRUE);
$xw->startDocument(NULL, "UTF-8");
$xw->writeDtdElement('sxe', '(elem1+, elem11, elem22*)');
$xw->writeDtdAttlist('sxe', 'id     CDATA  #implied');
$xw->startDtdElement('elem1');
$xw->text('elem2*');
$xw->endDtdElement();
$xw->startDtdAttlist('elem1');
$xw->text("attr1  CDATA  #required\n");
$xw->text('attr2  CDATA  #implied');
$xw->endDtdAttlist();
$xw->endDocument();
// Force to write and empty the buffer
$output = $xw->flush(true);
print $output;
?>