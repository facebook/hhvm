<?php

function VS($x, $y) {
  var_dump($x === $y);
  if ($x !== $y) { echo "Failed: $y\n"; echo "got: $x\n"; }
}

function VERIFY($x) {
  VS($x, true);
}

$xml = new XMLWriter();
$xml->openMemory();
var_dump($xml);
VERIFY($xml->setIndent(true));
VERIFY($xml->setIndentString("  "));
VERIFY($xml->startDocument("1.0", "utf-8"));

VERIFY($xml->startElement("node"));
VERIFY($xml->writeAttribute("name", "value"));
VERIFY($xml->startAttribute("name2"));
VERIFY($xml->endAttribute());
VERIFY($xml->writeElement("subnode", "some text"));
VERIFY($xml->endElement());

VERIFY($xml->startElementNS("fb", "node",
                                  "http://www.facebook.com/"));
VERIFY($xml->writeAttributeNS("fb", "attr",
                                    "http://www.facebook.com/", "value"));
VERIFY($xml->startAttributeNS("fb", "attr2",
                                    "http://www.facebook.com/"));
VERIFY($xml->endAttribute());
VERIFY($xml->writeElementNS("prefix", "name",
                                  "http://some.url/", 1337));
VERIFY($xml->startElement("node"));
VERIFY($xml->fullEndElement());
VERIFY($xml->endElement());

VERIFY($xml->startElement("node"));
VERIFY($xml->startCdata());
VERIFY($xml->text("Raw text"));
VERIFY($xml->endCdata());
VERIFY($xml->endElement());

VERIFY($xml->startElement("node"));
VERIFY($xml->writeCdata("More CDATA"));
VERIFY($xml->endElement());

VERIFY($xml->startComment());
VERIFY($xml->text("Comments"));
VERIFY($xml->endComment());

VERIFY($xml->writeComment("More comments"));

VERIFY($xml->startPi("lol"));
VERIFY($xml->endPi());
VERIFY($xml->writePi("php", "print 'Hello world!';"));

VERIFY($xml->writeRaw("<node>Raw XML</node>"));

VERIFY($xml->writeDTD("name", "publicID", "systemID", "subset"));
VERIFY($xml->startDTD("name", "publicID", "systemID"));
VERIFY($xml->endDTD());

VERIFY($xml->startDTDElement("name"));
VERIFY($xml->endDTDElement());
VERIFY($xml->writeDTDElement("name", "content"));

VERIFY($xml->startDTDAttlist("name"));
VERIFY($xml->endDTDAttlist());
VERIFY($xml->writeDTDAttlist("name", "content"));

VERIFY($xml->startDTDEntity("name", false));
VERIFY($xml->endDTDEntity());
VERIFY($xml->writeDTDEntity("name", "content", false, "publicid",
                                  "systemid", "ndataid"));

VERIFY($xml->endDocument());

var_dump($xml->flush());
var_dump($xml->outputMemory());
