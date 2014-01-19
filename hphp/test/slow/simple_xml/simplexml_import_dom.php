<?php

$string = <<<EOT
<?xml version="1.0" encoding="UTF-8"?>
<entry xmlns="http://www.w3.org/2005/Atom"
       xmlns:other="http://other.w3.org/other" >
  <id>uYG7-sPwjFg</id>
  <published>2009-05-17T18:29:31.000Z</published>
</entry>
EOT;

$doc = new DOMDocument;
$doc->loadXML($string);

$xml = simplexml_import_dom($doc);
$xml->registerXPathNamespace('atom', "http://www.w3.org/2005/Atom");
$nodes = $xml->xpath('//atom:entry/atom:published/text()');
foreach ($nodes as $node) {
  print $node;
}
