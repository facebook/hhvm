<?hh
/* $Id$ */
<<__EntryPoint>> function main(): void {
$xw = new XMLWriter();
$xw->openMemory();
$xw->setIndent(TRUE);
$xw->startDocument(NULL, "UTF-8");
$xw->writeDTDElement('sxe', '(elem1+, elem11, elem22*)');
$xw->writeDTDAttlist('sxe', 'id     CDATA  #implied');
$xw->startDTDElement('elem1');
$xw->text('elem2*');
$xw->endDTDElement();
$xw->startDTDAttlist('elem1');
$xw->text("attr1  CDATA  #required\n");
$xw->text('attr2  CDATA  #implied');
$xw->endDTDAttlist();
$xw->endDocument();
// Force to write and empty the buffer
$output = $xw->flush(true);
print $output;
}
