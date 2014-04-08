<?php

function main() {
  $x = new XMLWriter();
  $x->openMemory();
  $x->setIndent(true);
  $x->startDocument('1.0');
  $x->startElementNS(null, 'startNullPrefix', null);
  $x->startElementNS('', 'startEmptyPrefix', null);
  $x->writeElementNS(null, 'writeNullPrefix', null);
  $x->writeElementNS('', 'writeEmptyPrefix', null);
  $x->endElement();
  $x->endElement();
  $x->endDocument();
  var_dump($x->outputMemory(true));
}
main();
