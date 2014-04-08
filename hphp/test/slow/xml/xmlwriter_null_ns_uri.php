<?php

function main() {
  $x = new XMLWriter();
  $x->openMemory();
  $x->setIndent(true);
  $x->startDocument('1.0');
  $x->startElementNS(null, 'startNullURI', null);
  $x->startElementNS(null, 'startEmptyURI', '');
  $x->writeElementNS(null, 'writeNullURI', null);
  $x->writeElementNS(null, 'writeEmptyURI', '');
  $x->endElement();
  $x->endElement();
  $x->endDocument();
  var_dump($x->outputMemory(true));
}
main();
