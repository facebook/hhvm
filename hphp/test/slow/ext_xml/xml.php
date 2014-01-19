<?php
function VS($x, $y) {
  var_dump($x === $y);
  if ($x !== $y) { echo "Failed: $y\n"; }
}

VS(utf8_decode("abc \xc3\x80 def"), "abc \xc0 def");
VS(utf8_encode("abc \xc0 def"), "abc \xc3\x80 def");

// xml_parse_into_struct ..

$simple = "<para><note attrib1='foo'>simple&amp;note</note></para>";
$p = xml_parser_create();
xml_parse_into_struct($p, $simple, $vals, $index);
xml_parser_free($p);

var_dump($index["PARA"]);
var_dump($index["NOTE"]);
var_dump($vals[0]);
var_dump($vals[1]);
var_dump($vals[2]);


