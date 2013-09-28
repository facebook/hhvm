<?php

function VS($x, $y) {
  var_dump($x === $y);
  if ($x !== $y) { echo "Failed: $y\n"; echo "got: $x\n"; }
}

function VERIFY($x) {
  VS($x, true);
}

$xml = xmlwriter_open_memory();
VERIFY(xmlwriter_set_indent($xml, true));
VERIFY(xmlwriter_set_indent_string($xml, "  "));
VERIFY(xmlwriter_start_document($xml, "1.0", "utf-8"));
VERIFY(xmlwriter_start_element($xml, "node"));
VERIFY(xmlwriter_write_attribute($xml, "name", "value"));
VERIFY(xmlwriter_write_element($xml, "subnode", "some text"));
VERIFY(xmlwriter_end_element($xml));
VERIFY(xmlwriter_end_document($xml));
$out = xmlwriter_flush($xml);
VS($out, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<node name=\"value\">\n  <subnode>some text</subnode>\n</node>\n");
