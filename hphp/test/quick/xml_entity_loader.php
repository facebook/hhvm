<?php

$doc = new DOMDocument();
$doc->loadXML(
'<?xml version="1.0" encoding="UTF-8" ?>
<!DOCTYPE root [
    <!ENTITY test SYSTEM "data:text/plain;base64,aGVsbG8gd29ybGQ=">
]>
<root>&test;</root>',
LIBXML_DTDLOAD | LIBXML_NOENT
);

var_dump($doc->textContent);

?>
