<?php

$xw = xmlwriter_open_memory();
xmlwriter_start_document($xw, NULL, "UTF-8");
xmlwriter_start_dtd($xw, "root");
xmlwriter_write_dtd_entity($xw, "ent2", "val2");
xmlwriter_end_dtd($xw);
xmlwriter_start_element($xw, "root");
xmlwriter_end_document($xw);
print xmlwriter_flush($xw, true);
print "\n";

$xw = new XMLWriter();
$xw->openMemory();
$xw->startDocument(NULL, "UTF-8");
$xw->startDtd("root");
$xw->writeDtdEntity("c", NULL, 0, "-//W3C//TEXT copyright//EN", "http://www.w3.org/xmlspec/copyright.xml");
$xw->endDtd();
$xw->startElement("root");
$xw->endDocument();
print $xw->flush(true);

?>