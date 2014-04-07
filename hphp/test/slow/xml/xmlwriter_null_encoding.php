<?php

function main() {
  $x = new XMLWriter();
  $x->openMemory();
  $x->startDocument('1.0', null);
  $x->writeElement('root');
  $x->endDocument();
  var_dump($x->outputMemory(true));
}

main();
