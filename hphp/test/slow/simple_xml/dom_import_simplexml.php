<?php

function main() {
  $s = simplexml_load_string('<a><b>c</b></a>');
  $dom = dom_import_simplexml($s);
  print $dom->ownerDocument->saveXML();
}
main();
